// txt2mathjax.cpp : Defines the entry point for the console application.
//

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "stdlib.h"
#include "io.h"
#include "string.h"
#include <direct.h>
#include <fcntl.h>
#include <windows.h>
#include <process.h>


#define  TEST_TEMPLATE	"template_noad.txt"			// 見本用テンプレート名
#define  REAL_TEMPLATE	"template.txt"				// 本番用テンプレート名

#define	TEST_DIR_NAME	"converted_noad"			// 結果出力フォルダ名（見本用）
#define REAL_DIR_NAME	"converted"					// 結果出力フォルダ名（本番用）

#define DEFAULT_FOLDER	"c:\\physics\\site"

#define AD_MAX 4
#define AD_INTERVAL 3500

static char ad_foot[] = "<script async src=\"https://pagead2.googlesyndication.com/pagead/js/adsbygoogle.js\"></script>\n\
<!--foot-->\n\
<ins class = \"adsbygoogle\"\n\
	style = \"display:block\"\n\
	data-ad-client=\"ca-pub-2102965965451578\"\n\
	data-ad-slot=\"6936098044\"\n\
	data-ad-format=\"auto\"\n\
	data-full-width-responsive=\"true\"></ins>\n\
	<script>\n\
	(adsbygoogle = window.adsbygoogle || []).push({});\n\
</script>\n";

static char ad_relation[] = "<script async src=\"https://pagead2.googlesyndication.com/pagead/js/adsbygoogle.js\"></script>\n\
<ins class=\"adsbygoogle\"\n\
	style=\"display:block\"\n\
	data-ad-format=\"autorelaxed\"\n\
	data-ad-client=\"ca-pub-2102965965451578\"\n\
	data-ad-slot=\"1795778038\"></ins>\n\
<script>\n\
	(adsbygoogle = window.adsbygoogle || []).push({});\n\
</script>\n";


static char* test_template_buf;
static char* real_template_buf;
static char cur_dir[256];			// この実行ファイルのあるフォルダのフルパス */
static time_t template_time;		// テンプレートファイルのタイムスタンプ
static char test_product_dir[256];	// 結果出力フォルダ（見本用）のパス
static char real_product_dir[256];	// 結果出力フォルダ（本番用）のパス

static char field[256];
static char title[256];
static char heading[256];
static char comment[2048];
static char ogcomment[2048];
static char prev[256];
static char next[256];
static char create[256];
static char modify[256];

static int title_lines;		// タイトルの行数によってCSSを変更する仕組みのため
static int comment_lines;	// コメントの行数によってCSSを変更する仕組みのため

static char url_full[256];     // 処理中のファイルの置かれるURLを作って入れておく

static void make_directory(char* dir);
static int exist_dir(char* dir);
static char* read_file(char* fname);
static time_t get_time_stamp(char* fullpath);
static time_t get_html_time(char* dst_dir, char* fname);
static void end_wait(void);
static int check_dir(char* src_dir, char* dst_dir);
static int convert_main(char* src_txt, char* dst_dir);
static void make_fullpath_url(char* src_txt);
static void convert_add_ads(char* src, char* dst);
static void convert_sub(char* src, char* dst);
static void reset_params(void);
static char* set_param(char* dst, char* src);
static void split_param(char* param, char* param1, char* param2);
static int count_lines(char* src);
static void normalize_date(char* dst, char* src);
static void enable_inline_eq(char* dst, char* src);
static void note_indent(char* dst, char* src);
static void template_to_html(char* src, char* body, char* result);
static void url_encode(char* dst, char* src);
static void utf8_encode(char* dst, char* src);
static void remove_tags(char* dst, char* src);
static int write_html(char* file, char* html_buf);


int main(int argc, char* argv[]) {

	int rc;					// 結果
	char buf[2048];			// 汎用
	char buf2[512];
	char drive[32];
	char path[256];
	char name[128];
	char ext[32];			// 拡張子

	time_t time_a;
	time_t time_b;

	printf("HTMLコンバータ for EMANの物理学\n\n");

	// カレントフォルダを cur_dir に記録
	_splitpath(argv[0], drive, path, NULL, NULL);
	sprintf(cur_dir, "%s%s", drive, path);

	// カレントフォルダの下に結果出力用フォルダを作成
	sprintf(test_product_dir, "%s%s", cur_dir, TEST_DIR_NAME);
	sprintf(real_product_dir, "%s%s", cur_dir, REAL_DIR_NAME);
	make_directory((char*)"");

	// 見本用テンプレートの読み出し
	sprintf(buf, "%s%s", cur_dir, TEST_TEMPLATE);
	test_template_buf = read_file(buf);
	if (test_template_buf == NULL) {
		printf("テスト用テンプレート (%s) が開けません。\n", TEST_TEMPLATE);
		end_wait();
		return -1;
	}
	// テンプレートのタイムスタンプの読み出し
	time_a = get_time_stamp(buf);


	// 本番用テンプレートの読み出し
	sprintf(buf, "%s%s", cur_dir, REAL_TEMPLATE);
	real_template_buf = read_file(buf);
	if (real_template_buf == NULL) {
		free(test_template_buf);
		printf("本番用テンプレート (%s) が開けません。\n", REAL_TEMPLATE);
		end_wait();
		return -1;
	}
	// テンプレートのタイムスタンプの読み出し
	time_b = get_time_stamp(buf);

	// 後に更新された方の時刻を記録
	//printf("compare: %lld - %lld\n", time_a, time_b);
	template_time = ((time_a > time_b) ? time_a : time_b);

	if (argc == 2) {
		// ドラッグアンドドロップされた

		// ドロップされたファイル、フォルダの確認
		_splitpath(argv[1], NULL, NULL, name, ext);

		if (exist_dir(argv[1])) {

			// ドロップされたのはフォルダなので、フォルダとして作業開始
			sprintf(buf, "\\%s%s", name, ext);
			rc = check_dir(argv[1], buf);
			if (rc < 0) {
				printf("変換作業は中止されました。\n");
			}
			else {
				printf("変換作業は正常に完了しました。\n");
			}
		}
		else if (strcmp(ext, ".txt") == 0) {
			// 末尾をみて、txtなら、単独ファイルとして処理
			printf("単独ファイルとして処理を開始\n");
			rc = convert_main(argv[1], (char*)"");
			if (rc == 0) {
				printf("変換終了\n");
			}
			else {
				printf("エラー発生\n");
			}

		}
		else {
			// 関係ないものがドロップされた
			printf("txtファイルか、フォルダをドロップしてください。\n");
		}
	}
	else {
		// ドロップはされなかった。固定フォルダに対して作業を行う
		sprintf(buf, "%s", DEFAULT_FOLDER);
		sprintf(buf2, "%s", "\\site");
		rc = check_dir(buf, buf2);
		if (rc < 0) {
			printf("変換作業は中止されました。ぎゃー\n");
		}
		else {
			printf("変換作業は正常に完了しました。\n");
		}
	}

	free(test_template_buf);
	free(real_template_buf);
	end_wait();
	return 0;
}

/* ******************************************************* */
/*  出力フォルダ内に新しくディレクトリを作る               */
/*   　　dir は結果出力フォルダ以下の相対パス　            */
/*           末尾に￥を入れてはいけない  　　　　　　　　　*/
/* ******************************************************* */
static void make_directory(char* dir) {

	char buf[256];
	int rc;

	sprintf(buf, "%s%s", test_product_dir, dir);

	if (!exist_dir(buf)) {
		rc = _mkdir(buf);
		if (rc < 0) {
			printf("フォルダ作成に失敗しました。[%s]\n", buf);
			return;
		}
		printf("フォルダを新規作成しました。[%s]\n", buf);
	}

	sprintf(buf, "%s%s", real_product_dir, dir);

	if (!exist_dir(buf)) {
		rc = _mkdir(buf);
		if (rc < 0) {
			printf("フォルダ作成に失敗しました。[%s]\n", buf);
			return;
		}
		printf("フォルダを新規作成しました。[%s]\n", buf);
	}
}

/* ************************************************** */
/*      指定したディレクトリがあるかないかの調査      */
/*      true: あり      false: なし                   */
/* ************************************************** */
static int exist_dir(char* dir) {

	int hFile;
	struct _finddata_t c_file;
	char tmp[256];

	sprintf(tmp, "%s\\*.*", dir);
	hFile = _findfirst(tmp, &c_file);
	_findclose(hFile);

	if (hFile < 0) {
		return false;
	}
	return true;
}

/* ************************************************** */
/*    指定したファイルを開き、その内容のすべてを      */
/*   動的確保した領域に保存する。                     */
/*    領域の開放は呼び出した側が責任を持って行うこと。*/
/*    戻り値：　確保した領域の先頭アドレス            */
/*              NULL: 失敗                            */
/* ************************************************** */
static char* read_file(char* fname) {

	FILE* fp;
	char* buf;
	int fsize;
	int rc;

	fp = fopen(fname, "rb");
	if (fp == NULL) {
		printf("ファイルオープンに失敗[%s]\n", fname);
		return NULL;
	}
	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	buf = (char*)malloc(fsize + 512);		// 余裕を持って確保する
	if (buf == NULL) {
		printf("領域確保に失敗[%s] size==%d\n", fname, fsize);
		fclose(fp);
		return NULL;
	}

	rc = fread(buf, sizeof(char), fsize, fp);
	if (rc != fsize) {
		printf("読み出しに失敗[%s] rc==%d fsize==%d\n", fname, rc, fsize);
		fclose(fp);
		free(buf);
		return NULL;
	}

	fclose(fp);
	buf[fsize] = 0;

	return buf;
}

/* ************************************************** */
/*    指定したファイルのタイムスタンプを返す。        */
/*   もし見つからなければ 0 を返す。                  */
/*   テンプレートファイルのタイムスタンプ取得専用     */
/* ************************************************** */
static time_t get_time_stamp(char* fullpath) {

	long   hFile;
	struct _finddata_t c_file;

	hFile = _findfirst(fullpath, &c_file);
	if (hFile < 0) {
		return 0;
	}
	_findclose(hFile);

	//printf("time: %d\n", c_file.time_write);
	return c_file.time_write;
}

/* ************************************************** */
/*    生成するHTMLファイルのタイムスタンプを返す。    */
/*   もし見つからなければ 0 を返す。                  */
/*   fname は「*.txt」の形式の                        */
/*                     ファイル名でなければならない。 */
/* ************************************************** */
static time_t get_html_time(char* dst_dir, char* fname) {

	int len;
	char tmp[256];
	char target[256];
	time_t a;
	time_t b;

	len = strlen(fname);
	strcpy(tmp, fname);
	tmp[len - 4] = 0;

	// 見本用HTMLをチェック
	sprintf(target, "%s%s\\%s.html", test_product_dir, dst_dir, tmp);
	a = get_time_stamp(target);

	// 本番用HTMLをチェック
	sprintf(target, "%s%s\\%s.html", real_product_dir, dst_dir, tmp);
	b = get_time_stamp(target);

	return ((a < b) ? a : b); // より時刻が早い方を採用

}

/* ************************************************** */
/*  動作完了後にウィンドウを閉じないで待つための処理  */
/* ************************************************** */
static void end_wait(void) {
	printf("\nPush Return Key to Close\n");
	getchar();
}

/* ******************************************************* */
/*  フォルダの中をチェックする                             */
/*    src_dir の中にあるtxtファイルを見つけ変換作業する。  */
/*    フォルダがある場合にはさらに再帰的に呼び出す。       */
/*   src_dir の末尾に￥を入れてはいけない                  */
/*   dst_dir は結果出力フォルダ名を省いた部分のパス        */
/*           先頭には￥が要るが末尾に￥を入れてはいけない  */
/* ******************************************************* */
static int check_dir(char* src_dir, char* dst_dir) {

	/* ファイル一覧取得 */
	long   hFile;
	struct _finddata_t c_file;
	char target[256];
	char tmp_buf[256];
	char tmp_buf2[256];
	int len;

	// 同じディレクトリ構造にするために出力フォルダ側に同じディレクトリを作成
	make_directory(dst_dir);

	sprintf(target, "%s\\*.*", src_dir);
	hFile = _findfirst(target, &c_file);
	if (hFile < 0) {
		printf("フォルダ[%s]内に該当ファイル無し\n", src_dir);
		return -1;
	}

	//printf("%s\n", c_file.name);

	while (_findnext(hFile, &c_file) == 0) {

		if (c_file.name[0] == '.') continue;
		
		// draft という名前のフォルダは下書きが入っているので無視
		if (strcmp(c_file.name, "draft") == 0) continue;

		if (c_file.attrib & _A_SUBDIR) {
			/* フォルダなので再帰的に調べる */
			sprintf(tmp_buf, "%s\\%s", src_dir, c_file.name);
			sprintf(tmp_buf2, "%s\\%s", dst_dir, c_file.name);
			check_dir(tmp_buf, tmp_buf2);
			continue;
		}

		len = strlen(c_file.name);
		if (len < 4) continue;
		if (_strnicmp(c_file.name + len - 4, ".txt", 4) == 0) {
			time_t time;
			time = get_html_time(dst_dir, c_file.name);
			//printf("time:%lld %lld %lld\n", time, template_time, c_file.time_write);
			//if ((time < template_time) || (time < c_file.time_write)) { //　テンプレートを修正したら全部作り直す仕様はやめた
			if (time < c_file.time_write) {
				sprintf(tmp_buf, "%s\\%s", src_dir, c_file.name);
				convert_main(tmp_buf, dst_dir);
			}
			else {
				// 既にHTMLが作成されているので作業の必要なし
				//				printf("no need [%s]\n", tmp_buf);
			}
		}
		// printf("%s\n", c_file.name);
	}
	_findclose(hFile);
	return 0;

}

/* ************************************************** */
/*   テスト用の変換                                   */
/*    src_txt : 入力テキストファイルのフルパス        */
/*    dst_dir : 作成先フォルダ（固定部分は除く）      */
/*  dst_dir のフォルダ名の前には￥が要る　　　　　    */
/*  　　　　　　　空の場合には要らない　　　　　　    */
/*  dst_dir の末尾に￥を入れてはいけない 　　　　　   */
/* ************************************************** */
static int convert_main(char* src_txt, char* dst_dir) {

	char* src_buf;		// ソーステキストのデータの格納位置
	char* add_buf;		// 広告情報を追加後のデータの格納位置
	char* body_buf;		// 新たに作成されるHTML本文データの格納位置
	char* html_buf;		// HTMLの完成形データの格納位置
	int buf_size;		// 変換結果を入れるバッファのサイズ
	char txt_name[64];
	char fname_full[256];


	// ソースファイルをメモリに読み込む
	src_buf = read_file(src_txt);
	if (src_buf == NULL) {
		return -1;
	}

	// バッファとしてソースファイルの100倍程度を確保する
	buf_size = 100 * strlen(src_buf);

	// あらかじめ広告情報を挿入するためのバッファ
	add_buf = (char*)malloc(buf_size);
	if (add_buf == NULL) {
		printf("本文変換用バッファを確保できませんでした。\n");
		free(src_buf);
		return -1;
	}
	//memset(add_buf, 0, buf_size);

	// 変換結果を入れるバッファとしてソースファイルの100倍程度を確保
	body_buf = (char*)malloc(buf_size);
	if (body_buf == NULL) {
		printf("本文変換用バッファを確保できませんでした。\n");
		free(src_buf);
		free(add_buf);
		return -1;
	}
	//memset(body_buf, 0, buf_size);

	html_buf = (char*)malloc(buf_size);
	if (html_buf == NULL) {
		printf("テンプレート変換用バッファを確保できませんでした。\n");
		free(src_buf);
		free(add_buf);
		free(body_buf);
		return -1;
	}
	//memset(html_buf, 0, buf_size);


	// ソースファイルの名前を取り出しておく
	_splitpath(src_txt, NULL, NULL, txt_name, NULL);

	// OGP用にURLのフルパス作成
	make_fullpath_url(src_txt);

	// 見本用HTMLの生成・保存
	sprintf(fname_full, "%s%s\\%s.html", test_product_dir, dst_dir, txt_name);
	convert_sub(src_buf, body_buf);
	template_to_html(test_template_buf, body_buf, html_buf);
	write_html(fname_full, html_buf);

	// 本番用HTMLの生成・保存
	sprintf(fname_full, "%s%s\\%s.html", real_product_dir, dst_dir, txt_name);
	convert_add_ads(src_buf, add_buf);
	convert_sub(add_buf, body_buf);
	template_to_html(real_template_buf, body_buf, html_buf);
	write_html(fname_full, html_buf);

	printf("convert [%s]\n", src_txt);

	free(src_buf);
	free(add_buf);
	free(body_buf);
	free(html_buf);

	return 0;
}

/* ************************************************** */
/*  OGPのためにフルパスURLが必要なので                */
/*           原稿ファイルのフルパスから変換する       */
/*  src: ソーステキストのフルパス                     */
/* ************************************************** */
static void make_fullpath_url(char* src_txt) {

	int i;
	int len;
	char path[32];
	char tmp[256];
	
	strcpy( path, "c:\\physics\\site\\");
	len = strlen(path);
	strcpy(tmp, src_txt + len );
	len = strlen(tmp);
	tmp[len-4] = 0; // .txt を削除

	i = 0;
	while (tmp[i] != 0) {
		if (tmp[i] == '\\') {
			tmp[i] = '/';
			break;
		}
		i++;
	}
	sprintf( url_full, "https://eman-physics.net/%s.html", tmp);
}

/* ************************************************** */
/*    ソーステキストを変換にかける前に                */
/*                広告エリアを追加しておく            */
/*  src: ソース先頭                                   */
/*  dst: 改変後のデータを入れる領域                   */
/* ************************************************** */
static void convert_add_ads(char* src, char* dst) {

	int src_size;
	int start_idx;
	int prev_idx;
	int ad_num;
	int len;
	int left;

	src_size = strlen(src);
	start_idx = (int)src;
	prev_idx = start_idx;
	ad_num = 0;

	while (*src != 0) {
		if (((unsigned char)*src & 0xc0) == 0x80) {
			// UTF8の2バイト目以降の文字
			*(dst++) = *(src++);
			continue;
		}

		if (((unsigned char)*src & 0x80) == 0x80) {
			// 全角なので普通にコピー
			*(dst++) = *(src++);
			continue;
		}

		if (ad_num >= AD_MAX) {
			// 普通にコピー
			*(dst++) = *(src++);
			continue;
		}

#if 0
		if (*src == '\\') {		// エスケープ文字

			if (strncmp(src + 1, "section", 7) == 0) {
				// 広告挿入するか決める
				len = (int)src - prev_idx;                  // 前の節からの長さ 
				left = src_size - ((int)src - start_idx); // ここから終わりまでの長さ

				if ((len > AD_INTERVAL) && (left > AD_INTERVAL)) {
					ad_num++;
					prev_idx = (int)src;
					//printf("interval: %d\n", len);
					strcpy(dst, "\\ad1");
					dst += 4;
				}
			}
		}
#endif

		// 普通にコピー
		*(dst++) = *(src++);

	}

	*dst = 0;

	// 最後にフッター広告を付けるかどうかの判断
	len = (int)src - prev_idx;
	//printf("foot_interval: %d\n", len);
	if ((ad_num < AD_MAX) && (len > AD_INTERVAL)) {
		strcpy(dst, "\\ad2");
	}

}

/* ************************************************** */
/*    本文をルールに従ってHTMLに変換する              */
/*  src: ソース先頭                                   */
/*  dst: 改変後のデータを入れる領域                   */
/* ************************************************** */
static void convert_sub(char* src, char* dst) {

	int f_inline;
	int f_tex;
	int f_pre;
	int f_danraku;        /* 段落の途中であるかどうかを判定 */
	int f_after_eq;       /* 数式の後は次の段落が来る可能性があるが字下げで判断するしかないしpタグを入れないことにした */
	char param[8192];
	char tmp_buf[8192];
	char tmp_buf2[8192];
	char tmp_buf3[8192];
	char imgsrc[512];
	char altmsg[1024];
	char linkto[1024];

	f_inline = false;
	f_tex = false;
	f_pre = false;
	f_danraku = false;
	f_after_eq = false;

	title_lines = 1;
	comment_lines = 1;

	reset_params();

	while (*src != 0) {

		if (((unsigned char)*src & 0xc0) == 0x80) {
			// UTF8の2バイト目以降の文字なのでそのままコピーしてスルー
			*(dst++) = *(src++);
			continue;
		}

		if (f_inline) {
			if (*src == '$') {
				f_inline = false;
				strcpy(dst, "\\)</span>");
				dst += 9;
				src++;
				continue;
			}
			if (*src == '<') {
				strcpy(dst, "\\lt ");
				dst += 4;
				src++;
				continue;
			}
		}
		if (f_tex) {
			if (*src == '<') {
				if (strncmp(src, "</tex>", 6) == 0) {
					strcpy(tmp_buf, "\\end{align*}\n\\]\n</span>\n");
					src += 6;
					strcpy(dst, tmp_buf);
					dst += strlen(tmp_buf);
					f_tex = false;
					f_after_eq = true;
					continue;
				}
				strcpy(dst, "\\lt ");
				dst += 4;
				src++;
				continue;
			}
		}

		if (f_pre) {
			if (*src == '<') {
				if (strncmp(src, "</pre>", 6) == 0) {
					strcpy(tmp_buf, "\n</pre>");
					src += 6;
					strcpy(dst, tmp_buf);
					dst += strlen(tmp_buf);
					f_pre = false;
					continue;
				}
			}
		}

		if (f_inline || f_tex || f_pre) {
			*(dst++) = *(src++);
			continue;
		}

		// 改行を取り除く
		if (*src == '\r' || *src == '\n') {
			src++;
			continue;
		}

		if (((unsigned char)*src == 0xe3) && ((unsigned char)*(src + 1) == 0x80) && ((unsigned char)*(src + 2) == 0x80)) {
			//全角スペースを発見
			src += 3; // 排除する
			if (!f_danraku) {
				//段落先頭なので残したい。 隠しインデントに置き換え
				strcpy(tmp_buf, "\n<p>\n<span class=\"hidden_indent\"></span>");
				strcpy(dst, tmp_buf);
				dst += strlen(tmp_buf);
				f_danraku = true;
			} else if( f_after_eq ){
				strcpy(tmp_buf, "\n<span class=\"hidden_indent\"></span>");
				strcpy(dst, tmp_buf);
				dst += strlen(tmp_buf);
				f_after_eq = false;
			}
			continue;
		}

		f_after_eq = false;

		if (((unsigned char)*src & 0x80) == 0x80) { // 何らかの全角文字である

			f_after_eq = false;

			if (!f_danraku) {
				strcpy(dst, "\n<p>\n");
				dst += 4;
				f_danraku = true;
			}

			if (strncmp(src, u8"！", 3) == 0) {
				if (*(src + 3) == 0x20) { // ！マークの後の半角スペースは排除しておく
					src += 4;
				}
				else {
					src += 3;
				}
				strcpy(tmp_buf, u8"<span class=\"exclamation\">！</span>");
				strcpy(dst, tmp_buf);
				dst += strlen(tmp_buf);
				continue;
			}

			if (strncmp(src, u8"？", 3) == 0) {
				if (*(src + 3) == 0x20) { // ？マークの後の半角スペースは排除しておく
					src += 4;
				}
				else {
					src += 3;
				}
				strcpy(tmp_buf, u8"<span class=\"question\">？</span>");
				strcpy(dst, tmp_buf);
				dst += strlen(tmp_buf);
				continue;
			}

			if (strncmp(src, u8"、", 3) == 0) {
				src += 3;
				strcpy(tmp_buf, "<span class=\"punctuation\">,</span>");
				strcpy(dst, tmp_buf);
				dst += strlen(tmp_buf);
				continue;
			}

			if (strncmp(src, u8"。", 3) == 0) {
				src += 3;
				strcpy(tmp_buf, "<span class=\"period\">.</span>");
				strcpy(dst, tmp_buf);
				dst += strlen(tmp_buf);
				continue;
			}

			// その他の全角は普通にコピー
			*(dst++) = *(src++);
			continue;
		}

		if (*src == '\\') {		// エスケープ文字

			if (strncmp(src + 1, "field", 5) == 0) {
				src += 7;
				src = set_param(field, src);
			}
			else if (strncmp(src + 1, "title", 5) == 0) {
				src += 7;
				src = set_param(title, src);
				remove_tags(heading, title);
				title_lines = count_lines(title);
				if (title_lines > 2) title_lines = 2;
			}
			else if (strncmp(src + 1, "comment", 7) == 0) {
				src += 9;
				src = set_param(param, src);
				enable_inline_eq(comment, param);
				remove_tags(ogcomment, param);
				comment_lines = count_lines(comment);
				if (comment_lines > 3) comment_lines = 3;
			}
			else if (strncmp(src + 1, "prev", 4) == 0) {
				src += 6;
				src = set_param(prev, src);
			}
			else if (strncmp(src + 1, "next", 4) == 0) {
				src += 6;
				src = set_param(next, src);
			}
			else if (strncmp(src + 1, "create", 6) == 0) {
				src += 8;
				src = set_param(tmp_buf, src);
				normalize_date(create,tmp_buf);
			}
			else if (strncmp(src + 1, "modify", 6) == 0) {
				src += 8;
				src = set_param(tmp_buf, src);
				normalize_date(modify, tmp_buf);
			}
			else if (strncmp(src + 1, "section", 7) == 0) {
				src += 9;
				src = set_param(param, src);
				enable_inline_eq(tmp_buf, param);
				if (f_danraku) {
					strcpy(dst, "\n</p>");
					dst += 5;
					f_danraku = false;
				}
				if (tmp_buf[0] == 0) {
					sprintf(tmp_buf2, "\n<hr>");
				} else {
					sprintf(tmp_buf2, "\n<hr>\n<h2>%s</h2>", tmp_buf);
				}
				strcpy(dst, tmp_buf2);
				dst += strlen(tmp_buf2);
			}
			else if (strncmp(src + 1, "red", 3) == 0) {
				src += 5;
				src = set_param(param, src);
				note_indent(tmp_buf, param);
				sprintf(tmp_buf2, "<span class=\"attention\">%s</span>", tmp_buf);
				strcpy(dst, tmp_buf2);
				dst += strlen(tmp_buf2);
			}
			else if (strncmp(src + 1, "black", 5) == 0) {
				src += 7;
				src = set_param(param, src);
				note_indent(tmp_buf, param);
				sprintf(tmp_buf2, "<span class=\"attention2\">%s</span>", tmp_buf);
				strcpy(dst, tmp_buf2);
				dst += strlen(tmp_buf2);
			}
			else if (strncmp(src + 1, "amatxt", 6) == 0) {
				if (f_danraku) {
					strcpy(dst, "\n</p>");
					dst += 5;
					f_danraku = false;
				}

				src += 8;
				src = set_param(param, src);
				split_param(param, tmp_buf, tmp_buf2);
				sprintf(tmp_buf3, "\n<div class=\"amazon\">\n<table><tr><td>%s</td><td>%s\n</td></tr></table>\n</div>", tmp_buf, tmp_buf2);
				strcpy(dst, tmp_buf3);
				dst += strlen(tmp_buf3);
			}
			else if (strncmp(src + 1, "amazone", 7) == 0) {
				if (f_danraku) {
					strcpy(dst, "\n</p>");
					dst += 5;
					f_danraku = false;
				}

				src += 9;
				src = set_param(param, src);
				sprintf(tmp_buf, "<div class=\"amazon\">\n%s\n</div>", param);
				strcpy(dst, tmp_buf);
				dst += strlen(tmp_buf);
			}
			else if (strncmp(src + 1, "note", 4) == 0) {
				if (f_danraku) {
					strcpy(dst, "\n</p>");
					dst += 5;
					f_danraku = false;
				}

				src += 6;
				src = set_param(param, src);
				enable_inline_eq(tmp_buf, param);
				note_indent(tmp_buf2, tmp_buf);
				sprintf(tmp_buf, "\n<p>\n<span class=\"note\">\n%s\n</span>\n</p>", tmp_buf2);
				strcpy(dst, tmp_buf);
				dst += strlen(tmp_buf);
			}
			else if (strncmp(src + 1, "revise", 6) == 0) {
				if (f_danraku) {
					strcpy(dst, "\n</p>");
					dst += 5;
					f_danraku = false;
				}

				src += 8;
				src = set_param(param, src);
				enable_inline_eq(tmp_buf, param);
				note_indent(tmp_buf2, tmp_buf);
				sprintf(tmp_buf, "\n<p>\n<span class=\"revise\">\n%s\n</span>\n</p>", tmp_buf2);
				strcpy(dst, tmp_buf);
				dst += strlen(tmp_buf);
			}
			else if (strncmp(src + 1, "image", 5) == 0) {
				if (f_danraku) {
					strcpy(dst, "\n</p>\n");
					dst += 6;
					f_danraku = false;
				}
				src += 7;
				src = set_param(param, src);
				split_param(param, imgsrc, tmp_buf);
				split_param(tmp_buf, altmsg, linkto);
				if (altmsg[0] == 0 && linkto[0] == 0) {
					if (linkto[0] == 0) {
						sprintf(tmp_buf, "\n<div class=\"figures\"><img src=\"%s\"></div>\n", imgsrc);
					}
					else {
						sprintf(tmp_buf, "\n<div class=\"figures\"><a href=\"%s\"><img src=\"%s\"></a></div>\n", linkto,imgsrc);
					}
				}
				else {
					if (linkto[0] == 0) {
						sprintf(tmp_buf, "\n<div class=\"figures\"><img src=\"%s\" alt=\"%s\"></div>\n", imgsrc, altmsg);
					}
					else {
						sprintf(tmp_buf, "\n<div class=\"figures\"><a href=\"%s\"><img src=\"%s\" alt=\"%s\"></a></div>\n", linkto, imgsrc, altmsg);
					}
				}
				strcpy(dst, tmp_buf);
				dst += strlen(tmp_buf);
			}
			else if (strncmp(src + 1, "ad1", 3) == 0) {
				if (f_danraku) {
					strcpy(dst, "\n</p>\n");
					dst += 6;
					f_danraku = false;
				}
				src += 4;
				sprintf(tmp_buf, "<hr>\n");
				strcat(tmp_buf, ad_foot);
				strcpy(dst, tmp_buf);
				dst += strlen(tmp_buf);
			}
			else if (strncmp(src + 1, "ad2", 3) == 0) {
				if (f_danraku) {
					strcpy(dst, "\n</p>\n");
					dst += 6;
					f_danraku = false;
				}
				src += 4;
				sprintf(tmp_buf, "<hr>\n");
				strcat(tmp_buf, ad_relation);
				strcpy(dst, tmp_buf);
				dst += strlen(tmp_buf);
			}
			else {
				// ￥マークの次の文字は普通に採用する
				src++;
				*(dst++) = *(src++);
			}
			continue;
		}
		else if (*src == '$') {	// インライン数式
			f_inline = true;
			strcpy(dst, "<span class=\"eqinline\">\\(");
			dst += 25;
			src++;
			continue;
		}
		else if (*src == '<') {	// 数式の可能性
			if (strncmp(src, "<tex>\n", 6) == 0) {
				if (!f_danraku) {
					strcpy(dst, "\n<p>\n");
					dst += 5;
					f_danraku = true;
				}
				src += 6;
				strcpy(tmp_buf, "\n<span class=\"display\">\n\\[\n\\begin{align*}\n");
				strcpy(dst, tmp_buf);
				dst += strlen(tmp_buf);
				f_tex = true;
				continue;
			}
			if (strncmp(src, "<pre>", 5) == 0) {
				if (!f_danraku) {
					strcpy(dst, "\n<p>\n");
					dst += 5;
					f_danraku = true;
				}
				src += 5;
				strcpy(tmp_buf, "\n<pre>\n");
				strcpy(dst, tmp_buf);
				dst += strlen(tmp_buf);
				f_pre = true;
				continue;
			}
			if (strncmp(src, "<P>", 3) == 0 || strncmp(src, "<p>", 3) == 0) {
				src += 3;
				if (f_danraku) {
					strcpy(dst, "\n</p>");
					dst += 5;
					f_danraku = false;
				}
				continue;
			}
		}
		else if (*src == '%') { // 行の終わりまではコメント
								// 次の行の初めまで飛ばす
			while ((*src != 0) && (*src++ != '\n'));
			continue;
		}

		// それ以外のなんらかの半角
		if (!f_danraku) {
			strcpy(tmp_buf, "\n<p>\n");
			strcpy(dst, tmp_buf);
			dst += strlen(tmp_buf);
			f_danraku = true;
		}

		// 普通にコピー
		*(dst++) = *(src++);
	}

	if (f_danraku) {
		strcpy(dst, "\n</p>\n");
		dst += 6;
	}
	*dst = 0;	// 終端のヌル（念のため）

}

/* ************************************************** */
/*    各種パラメータをクリアする。            */
/* ************************************************** */
static void reset_params(void) {

	field[0] = 0;
	heading[0] = 0;
	title[0] = 0;
	comment[0] = 0;
	prev[0] = 0;
	next[0] = 0;
	create[0] = 0;
	modify[0] = 0;
}

/* ************************************************** */
/*      } が現れるまでの内容を dst にコピーする。     */
/*   } の次の文字の位置をポインタで返す。             */
/* ************************************************** */
static char* set_param(char* dst, char* src) {

	while (*src != '}') {

		if (*src == 0) {		// 普通はないと思うが、終端なのでループを抜ける
			*dst = 0;		// 終端の 0 
			return src;
		}

		*(dst++) = *(src++);
	}
	*dst = 0; // 終端の 0 
	return (src + 1);
}

/* ************************************************** */
/*   改行 <br> が幾つあるかを数える。N+1を返す        */
/* ************************************************** */
static int count_lines(char* src) {
	int n;

	n = 1;

	while (*src != 0) {
		if (strncmp(src, "<br>", 4) == 0) {
			n++;
		}
		src++;
	}

	return n;
}

/* ************************************************** */
/*   文字列を , を区切り文字として二つに分割する。    */
/* ************************************************** */
static void split_param(char* param, char* param1, char* param2) {

	strcpy(param1, param);

	while (*param1 != 0) {
		if (*param1 == ',') {
			*param1 = 0;
			strcpy(param2, param1 + 1);
			return;
		}
		param1++;
	}
	*param2 = 0;
}

/* ************************************************** */
/*   create や modify の時刻表記を正規化する          */
/*  記事に入れるのは日付だけなので時刻部分は削除して　*/
/*  　時刻の中にある位置合わせの0を省く               */
/* ************************************************** */
static void normalize_date(char* dst, char* src) {

	int year = 0;
	int month = 0;
	int day = 0;
	int hour = 0;
	int min = 0;
	int sec = 0;

	sscanf(src, "%d/%d/%d %d:%d:%d", &year, &month, &day, &hour, &min, &sec);
	if (year == 0 || month == 0 || day == 0) {
		dst[0] = 0;
	}
	else {
		sprintf(dst, "%d/%d/%d", year, month, day);
	}
}

/* ************************************************** */
/*   \note や \section や                             */
/*       \comment 内での数式の使用を可能にする        */
/*   $ と $ とで挟まれた部分を \( と \) に書き換える  */
/* ************************************************** */
static void enable_inline_eq(char* dst, char* src) {

	int f_inline = false;

	while (*src != 0) {

		if (*src == '$') {
			if (f_inline) {
				f_inline = false;
				strcpy(dst, "\\)</span>");
				dst += strlen(dst);
				src++;
			}
			else {
				f_inline = true;
				strcpy(dst, "<span class=\"eqinline\">\\(");
				dst += strlen(dst);
				src++;
			}
			continue;
		}
		*(dst++) = *(src++);
	}
	*dst = 0; // 終端の 0 
}

/* ************************************************** */
/*   \note や \red 中でのインデント制御                       */
/*   　改行や全角空白を排除する                       */
/* ************************************************** */
static void note_indent(char* dst, char* src) {

	// 「補足：」などのようにインデントしない方がいいような使い方をしている箇所も多いので、
	// 機械的な判断で強制インデントをしないようにした。
	// 全角空白を少し大きめの空白に置き換えることでとりあえず対応

	char tmp_buf[256];

	while (*src != 0) {

		// 改行を取り除く
		if (*src == '\r' || *src == '\n') {
			src++;
			continue;
		}

		if (((unsigned char)*src == 0xe3) && ((unsigned char)*(src + 1) == 0x80) && ((unsigned char)*(src + 2) == 0x80)) {
			//全角スペースを発見
			src += 3;
			strcpy(tmp_buf, "<span class=\"hidden_indent\"></span>");
			strcpy(dst, tmp_buf);
			dst += strlen(tmp_buf);
			continue;
		}

		if (((unsigned char)*src & 0x80) == 0x80) { // 何らかの全角文字である

			if (strncmp(src, u8"！", 3) == 0) {
				if (*(src + 3) == 0x20) { // ！マークの後の半角スペースは排除しておく
					src += 4;
				}
				else {
					src += 3;
				}
				strcpy(tmp_buf, u8"<span class=\"exclamation\">！</span>");
				strcpy(dst, tmp_buf);
				dst += strlen(tmp_buf);
				continue;
			}

			if (strncmp(src, u8"？", 3) == 0) {
				if (*(src + 3) == 0x20) { // ？マークの後の半角スペースは排除しておく
					src += 4;
				}
				else {
					src += 3;
				}
				strcpy(tmp_buf, u8"<span class=\"question\">？</span>");
				strcpy(dst, tmp_buf);
				dst += strlen(tmp_buf);
				continue;
			}

			if (strncmp(src, u8"、", 3) == 0) {
				src += 3;
				strcpy(tmp_buf, "<span class=\"punctuation\">,</span>");
				strcpy(dst, tmp_buf);
				dst += strlen(tmp_buf);
				continue;
			}

			if (strncmp(src, u8"。", 3) == 0) {
				src += 3;
				strcpy(tmp_buf, "<span class=\"period\">.</span>");
				strcpy(dst, tmp_buf);
				dst += strlen(tmp_buf);
				continue;
			}
		}

		*(dst++) = *(src++);
	}
	*dst = 0; // 終端の 0 
}

/* ************************************************** */
/*    テンプレートを元に仕上げ作業をする。            */
/*   src: テンプレートファイルのバッファ              */
/*   body: 本文の入ったバッファ                       */
/*   result: 結果を入れるバッファ　　　　　　　　　　 */
/* ************************************************** */
static void template_to_html(char* src, char* body, char* result ) {

	const char* str_prev = u8"前の記事へ";
	const char* str_next = u8"次の記事へ";
	char tmp[512];

	//printf( "field:[%s]\n", field );
	//printf( "title:[%s]\n", title );
	//printf( "comment:[%s]\n", comment );
	//printf( "prev:[%s]\n", prev );
	//printf( "next:[%s]\n", next );
	
	while (*src != 0) {
		if (*src == '@') {
			if (strncmp(src + 1, "heading", 7) == 0) {
				src += 8;
				sprintf(tmp, "%s%s%s", heading, u8" - EMANの", field);
				strcpy(result, tmp);
				result += strlen(tmp);
			}
			else if (strncmp(src + 1, "field", 5) == 0) {
				src += 6;
				strcpy(result, field);
				result += strlen(field);
			}
			else if (strncmp(src + 1, "ogtitle", 7) == 0) {
				src += 8;
				sprintf(tmp, "%s%s%s", field, u8"：", heading);
				strcpy(result, tmp);
				result += strlen(tmp);
			}
			else if (strncmp(src + 1, "url", 3) == 0) {
				src += 4;
				strcpy(result, url_full);
				result += strlen(url_full);
			}
			else if (strncmp(src + 1, "enc_url", 7) == 0) {
				src += 8;
				url_encode(tmp, url_full);
				strcpy(result, tmp);
				result += strlen(tmp);
			}
			else if (strncmp(src + 1, "enc_title", 9) == 0) {
				src += 10;
				utf8_encode(tmp, title);
				strcpy(result, tmp);
				result += strlen(tmp);
			}
			else if (strncmp(src + 1, "enc_field", 9) == 0) {
				src += 10;
				utf8_encode(tmp, field);
				strcpy(result, tmp);
				result += strlen(tmp);
			}
			else if (strncmp(src + 1, "title", 5) == 0) {
				src += 6;
				strcpy(result, title);
				result += strlen(title);
			}
			else if (strncmp(src + 1, "titl2", 5) == 0) {
				src += 6;
				strcpy(result, heading);
				result += strlen(heading);
			}
			else if (strncmp(src + 1, "comment", 7) == 0) {
				src += 8;
				strcpy(result, comment);
				result += strlen(comment);
			}
			else if (strncmp(src + 1, "titlid", 6) == 0) {
				src += 7;
				if (title_lines == 2){
					strcpy(result, "2");
					result ++;
				}
			}
			else if (strncmp(src + 1, "comeid", 6) == 0) {
				src += 7;
				if (title_lines != 1 || comment_lines != 1){
					sprintf(result, "%d%d", title_lines, comment_lines);
					result += 2;
				}
			}
			else if (strncmp(src + 1, "ogcomment", 9) == 0) {
				src += 10;
				strcpy(result, ogcomment);
				result += strlen(ogcomment);
			}
			else if (strncmp(src + 1, "body", 4) == 0) {
				src += 5;
				strcpy(result, body);
				result += strlen(body);
			}
			else if (strncmp(src + 1, "ret_cont", 8) == 0) {
				src += 9;
				strcpy(result, field);
				result += strlen(field);
			}
			else if (strncmp(src + 1, "prev", 4) == 0) {
				src += 5;
				if (prev[0] == 0) {
					sprintf(tmp, "<font color=\"gray\">%s</font>", str_prev);
				}
				else {
					sprintf(tmp, "<A href=\"./%s.html\">%s</A>", prev, str_prev);
				}
				strcpy(result, tmp);
				result += strlen(tmp);
			}
			else if (strncmp(src + 1, "next", 4) == 0) {
				src += 5;
				if (next[0] == 0) {
					sprintf(tmp, "<font color=\"gray\">%s</font>", str_next);
				}
				else {
					sprintf(tmp, "<A href=\"./%s.html\">%s</A>", next, str_next);
				}
				strcpy(result, tmp);
				result += strlen(tmp);
			}
			else if (strncmp(src + 1, "create", 6) == 0) {
				src += 7;
				sprintf(tmp, u8"作成：%s", create);
				strcpy(result, tmp);
				result += strlen(tmp);
			}
			else if (strncmp(src + 1, "modify", 6) == 0) {
				src += 7;
				if (modify[0] != 0) {
					sprintf(tmp, u8"<br>更新：%s", modify);
					strcpy(result, tmp);
					result += strlen(tmp);
				}
			}
			else {
				*(result++) = *(src++);
			}


		}
		else {
			*(result++) = *(src++);
		}
	}

	*result = 0;
}

/* ************************************************** */
/*   ごく簡易的なURLエンコーディング      */
/* ************************************************** */
static void url_encode(char* dst, char* src) {

	while (*src != 0) {

		if (*src == ':') {
			src++;
			strcpy(dst, "%3A");
			dst += 3;
			continue;
		}

		if (*src == '/') {
			src++;
			strcpy(dst, "%2F");
			dst += 3;
			continue;
		}

		*dst++ = *src++;
	}

	*dst = 0;

}

/* ************************************************** */
/*   ごく簡易的なURLエンコーディング      */
/* ************************************************** */
static void utf8_encode(char* dst, char* src) {

	while (*src != 0) {

		if ((*src & 0x80) == 0) {
			if ((*src >= 0x30 && *src <= 0x39) ||	/* [0-9] */
				(*src >= 0x41 && *src <= 0x5A) ||	/* [A-Z] */
				(*src >= 0x61 && *src <= 0x7A)) {	/* [a-z] */
				*dst++ = *src++;
				continue;
			}
		}  

		sprintf(dst, "%%%02X", (unsigned char)*src);
		dst += 3;
		src++;
	}

	*dst = 0;

}

/* ************************************************** */
/*    src文字列からHTMLタグを削除したものを      */
/*   dstへと入れる。                  */
/* ************************************************** */
static void remove_tags(char* dst, char* src) {

	int f_tag;

	f_tag = false;

	while (*src != 0) {

		if (*src == '>') {
			f_tag = false;
			src++;
			continue;
		}

		if (f_tag) {
			src++;
			continue;
		}

		if (*src == '<') {
			f_tag = true;
			src++;
			continue;
		}

		*dst++ = *src++;
	}

	*dst = 0;
}

/* ************************************************** */
/*   変換後HTMLデータの書き込み                                   */
/*    src_txt : 入力テキストファイル                  */
/*    dst_dir : 作成先フォルダ（固定部分は除く）      */
/*  dst_dir のフォルダ名の前には￥が要る　　　　　    */
/*  　　　　　　　空の場合には要らない　　　　　　    */
/*  dst_dir の末尾に￥を入れてはいけない 　　　　　   */
/* ************************************************** */
static int write_html(char* file, char* html_buf) {

	FILE* fp;

	fp = fopen(file, "wb");
	if (fp == NULL) {
		printf("ファイルが作成できません。[%s]\n", file);
		return -1;
	}

	// 本文を書き込む
	fwrite(html_buf, sizeof(char), strlen(html_buf), fp);

	fclose(fp);

	return 0;
}
