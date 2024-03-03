// make_sitemap.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
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
#include <time.h>

#define DEFAULT_FOLDER	"c:\\physics\\site"
#define SITEMAP_TEMPLATE "sitemap_dat.txt"
#define RESULT_FILE_NAME "sitemap.xml"

#define PRIORITY_LOW	0.5f
#define PRIORITY_HIGH	0.8f


static char cur_dir[256];			// この実行ファイルのあるフォルダのフルパス */

static char create[256];
static char modify[256];

static char url_full[256];     // 処理中のファイルの置かれるURLを作って入れておく

static char result_buf[300000];
static int  result_idx;

static char time_stamp[50];
static float priority;

static int exist_dir(char* dir);
static char* read_file(char* fname);
static void end_wait(void);
static int check_dir(char* src_dir);
static int convert_main(char* src_txt);
static void make_fullpath_url(char* src_txt);
static void convert_sub(char* src);
static char* set_param(char* dst, char* src);
static void normalize_date(char* dst, char* src);
static int make_time_stamp(void);
static void decide_priority(int year, int month, int day);
static int write_sitemap(char* file, char* buf);


int main(int argc, char* argv[]) {

	int rc;					// 結果
	char buf[2048];			// 汎用
	char drive[32];
	char path[256];
	//char name[128];
	//char ext[32];			// 拡張子
	char* template_buf;


	printf("SITEMAPジェネレータ for EMANの物理学\n\n");

	// カレントフォルダを cur_dir に記録
	_splitpath(argv[0], drive, path, NULL, NULL);
	sprintf(cur_dir, "%s%s", drive, path);

	// 固定データの読み出し
	sprintf(buf, "%s%s", cur_dir, SITEMAP_TEMPLATE);
	template_buf = read_file(buf);
	if (template_buf == NULL) {
		printf("テンプレート (%s) が開けません。\n", SITEMAP_TEMPLATE);
		end_wait();
		return -1;
	}
	result_idx = strlen(template_buf);
	strcpy(result_buf, template_buf);
	free(template_buf);
	

	// 固定フォルダに対して作業を行う
	sprintf(buf, "%s", DEFAULT_FOLDER);
	rc = check_dir(buf);
	if (rc < 0) {
		printf("作業は中止されました。ぎゃー\n");
	}

	strcpy(result_buf + result_idx, "</urlset>\n");
	sprintf(buf, "%s%s", cur_dir, RESULT_FILE_NAME);
	write_sitemap(buf, result_buf);

	printf("作業は正常に完了しました。\n");

	end_wait();
	return 0;
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
static int check_dir(char* src_dir) {

	/* ファイル一覧取得 */
	long   hFile;
	struct _finddata_t c_file;
	char target[256];
	char tmp_buf[256];
	//char tmp_buf2[256];
	int len;
	int rc;

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
			//sprintf(tmp_buf2, "%s\\%s", dst_dir, c_file.name);
			rc = check_dir(tmp_buf);
			if (rc < 0) {
				_findclose(hFile);
				return -1;
			}
			continue;
		}

		len = strlen(c_file.name);
		if (len < 4) continue;
		if (_strnicmp(c_file.name + len - 4, ".txt", 4) == 0) {
			sprintf(tmp_buf, "%s\\%s", src_dir, c_file.name);

			// 除外リスト
			// printf("aaa: [%s][%s]\n", src_dir, c_file.name);
			if (strcmp(tmp_buf, "c:\\physics\\site\\analytic\\a_test.txt") == 0) {
				printf("除外します [%s]\n", tmp_buf);
				continue;
			}

			rc = convert_main(tmp_buf);
			if (rc < 0) {
				_findclose(hFile);
				return -1;
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
static int convert_main(char* src_txt) {

	char* src_buf;		// ソーステキストのデータの格納位置
	char txt_name[64];
	//char fname_full[256];
	char buf[1024];
	int rc;

	// ソースファイルをメモリに読み込む
	src_buf = read_file(src_txt);
	if (src_buf == NULL) {
		return -1;
	}

	// ソースファイルの名前を取り出しておく
	_splitpath(src_txt, NULL, NULL, txt_name, NULL);

	// URLのフルパス作成
	make_fullpath_url(src_txt);

	convert_sub(src_buf);
	free(src_buf);

	rc = make_time_stamp();
	if (rc < 0) {
		return -1;
	}

	sprintf(buf, "<url>\n<loc>%s</loc>\n<lastmod>%s</lastmod>\n<priority>%.1f</priority>\n</url>\n", url_full, time_stamp, priority);
	strcpy(result_buf + result_idx, buf);
	result_idx += strlen(buf);

	printf("check %s\n", url_full);

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

	strcpy(path, "c:\\physics\\site\\");
	len = strlen(path);
	strcpy(tmp, src_txt + len);
	len = strlen(tmp);
	tmp[len - 4] = 0; // .txt を削除

	i = 0;
	while (tmp[i] != 0) {
		if (tmp[i] == '\\') {
			tmp[i] = '/';
			break;
		}
		i++;
	}
	sprintf(url_full, "https://eman-physics.net/%s.html", tmp);
}


/* ************************************************** */
/*    本文をルールに従ってHTMLに変換する              */
/*  src: ソース先頭                                   */
/* ************************************************** */
static void convert_sub(char* src) {

	create[0] = 0;
	modify[0] = 0;

	while (*src != 0) {

		if (((unsigned char)*src & 0xc0) == 0x80) {
			// UTF8の2バイト目以降の文字なのでスルー
			src++;
			continue;
		}

		// 改行をスルー
		if (*src == '\r' || *src == '\n') {
			src++;
			continue;
		}

		if (((unsigned char)*src & 0x80) == 0x80) {
			// 何らかの全角文字である
			src += 3; // 排除する
			continue;
		}

		if (*src == '\\') {		// エスケープ文字

			if (strncmp(src + 1, "create", 4) == 0) {
				src += 8;
				src = set_param(create, src);
			}
			else if (strncmp(src + 1, "modify", 4) == 0) {
				src += 8;
				src = set_param(modify, src);
			}
			else if (strncmp(src + 1, "section", 7) == 0) {
				// ここまで読んだら終わってもいい
				return;
			}
			src++;
			continue;
		}

		if (*src == '%') { // 行の終わりまではコメント
								// 次の行の初めまで飛ばす
			while ((*src != 0) && (*src++ != '\n'));
			continue;
		}

		// それ以外のなんらかの半角
		src++;
	}
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
static int make_time_stamp( void ) {

	int year = 0;
	int month = 0;
	int day = 0;
	int hour = 0;
	int min = 0;
	int sec = 0;

	if (create[0] == 0) {
		printf("作成日時が検出できませんでした\n");
		return -1;
	}

	if (modify[0] == 0) {
		sscanf(create, "%d/%d/%d %d:%d:%d", &year, &month, &day, &hour, &min, &sec);
	} else {
		sscanf(modify, "%d/%d/%d %d:%d:%d", &year, &month, &day, &hour, &min, &sec);

	}
	if (year == 0 || month == 0 || day == 0) {
		printf("作成日時の記述に異常\n");
		return -1;
	}

	sprintf(time_stamp, "%d-%02d-%02dT%02d:%02d:%02d+09:00", year, month, day, hour, min, sec);

	decide_priority(year, month, day);
	return 0;
}

/* ************************************************** */
/*   約2ヵ月以内かどうかの判定          */
/* ************************************************** */
static void decide_priority(int year, int month, int day) {

	time_t now;
	struct tm *tm;

	now = time(NULL);
	now -= 2 * 30 * 24 * 3600; // 約2か月前の値にする
	tm = localtime(&now);
	
	//printf("%d/%d/%d %d\n", tm->tm_year, tm->tm_mon, tm->tm_mday, tm->tm_yday);

	if (year < tm->tm_year + 1900) {
		priority = PRIORITY_LOW;
		return;
	}

	if (year > tm->tm_year + 1900) {
		priority = PRIORITY_HIGH;
		return;
	}

	if (month < tm->tm_mon) {
		priority = PRIORITY_LOW;
		return;
	}

	if (month > tm->tm_mon) {
		priority = PRIORITY_HIGH;
		return;
	}

	if (day < tm->tm_mday) {
		priority = PRIORITY_LOW;
		return;
	}

	priority = PRIORITY_HIGH;

}

/* ************************************************** */
/*   完成ファイル書き込み                             */
/* ************************************************** */
static int write_sitemap(char* file, char* buf) {

	FILE* fp;

	fp = fopen(file, "wb");
	if (fp == NULL) {
		printf("ファイルが作成できません。[%s]\n", file);
		return -1;
	}

	// 本文を書き込む
	fwrite(buf, sizeof(char), strlen(buf), fp);

	fclose(fp);

	return 0;
}
