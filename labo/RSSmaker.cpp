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
#include <time.h>



#define DEFAULT_FOLDER	"c:\\physics\\site"
#define RSS_FNAME		"c:\\physics\\rss.xml"

static char rss_head[] = u8"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n\
<rss version=\"2.0\">\n\
<channel>\n\
\n\
<title>EMANの物理学</title>\n\
<link>http://eman-physics.net/</link>\n\
<description>物理学の基礎分野の解説</description>\n\
<language>ja</language>\n";

static char week_str[7][8] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static char mon_str[12][8] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

static int total;
static int read_total;

static unsigned long t_int[1000];
static char field[1000][16];  // 分野を表すフォルダ名
static char fname[1000][32];  // ファイル名
static int index[1000];

static char cur_dir_name[16];

static char title[256];
static char link[256];
static char category[256];
static char desc[2048];
static char date[256];


static char url_full[256];     // 処理中のファイルの置かれるURLを作って入れておく

static void sort(void);
static void write_rss(void);
static int exist_dir(char* dir);
static char* read_file(char* fname);
static void end_wait(void);
static int check_dir(char* src_dir);
static int convert_main(char* src_txt);
static void search_date(char* src);
static int get_data(int num);
static void search_article(char* src);
static int u8len(char* src);
static char* set_param(char* dst, char* src);
static unsigned long int_date(char* src);
static int make_date_str(char* src);
static void remove_tags(char* dst, char* src);

static void make_alt_string(char* equation, char* alt);
static void alt_alias(char* src, char* dst);
static void alt_vec(char* src, char* dst);
static void alt_index(char* src, char* dst);
static void alt_sqrt(char* src, char* dst);
static void alt_frac(char* src, char* dst);
static int alt_frac_sub(char* src, char* dst, int mode);
static int alt_count_variables(char* src);
static int alt_exist_paren(char* src);
static void alt_braket(char* src, char* dst);
static void alt_greek(char* src, char* dst);
static void alt_space(char* src, char* dst);

int main(int argc, char* argv[]) {

	int rc;					// 結果
	char buf[256];			// 汎用


	printf("RSSジェネレーター for EMANの物理学\n\n");

	total = 0;
	read_total = 0;

	sprintf(buf, "%s", DEFAULT_FOLDER);
	rc = check_dir(buf);
	if (rc < 0) {
		printf("データ収集は中止されました。ぎゃー\n");
	}
	else {
		printf("データ収集が完了しました。(%d/%d)\n", total, read_total);
	}

	if (total < 50) {
		printf("記事の数が少なすぎます\n");
		end_wait();
		return 1;
	}

	// 時間順ソート
	sort();

	/* デバグ用　ソート結果の確認
	int i;
	printf("after sort\n");
	for (i = 0; i < total; i++) {
		printf("sort [%d]\n", t_int[index[i]]);
	}
	*/

	write_rss();

	end_wait();
	return 0;
}

/* ******************************************************* */
/*  並べ替える               */
/* ******************************************************* */
static void sort(void) {

	int i;
	int j;
	int temp;

	for (i = 0; i < total; i++) {
		index[i] = i;
	}

	for(i = 0; i < total; i++) {
		for (j = total - 1; j > i; j--) {
			if (t_int[index[j]] > t_int[index[j - 1]]) {
				temp = index[j];
				index[j] = index[j - 1];
				index[j - 1] = temp;
			}
		}
	}
}

/* ******************************************************* */
/*  結果を書き込む               */
/* ******************************************************* */
static void write_rss(void) {

	FILE* fp;
	char buf[2048];
	int i;
	int rec_total;
	int rc;

	fp = fopen(RSS_FNAME, "wb");
	if (fp == NULL) {
		printf("ファイルが作成できません。[%s]\n", RSS_FNAME);
		return;
	}
	
	fwrite(rss_head, sizeof(char), strlen(rss_head), fp);

	//strcpy( date_str, "Thu, 22 Oct 2020 22:30:00 +0900");
	get_data(0);
	sprintf(buf, "<lastBuildDate>%s</lastBuildDate>\n\n", date);
	fwrite(buf, sizeof(char), strlen(buf), fp);

	i = 0;
	rec_total = 0;

	while(rec_total <30) {
		rc = get_data(i++);
		if (rc < 0) {
			continue;
		}
		sprintf(buf, "<item>\n<title>%s</title>\n<link>%s</link>\n<category>%s</category>\n<description>%s</description>\n<pubDate>%s</pubDate>\n</item>\n\n",
			title, link, category, desc, date);
		fwrite(buf, sizeof(char), strlen(buf), fp);
		rec_total++;
	}

	sprintf(buf, "</channel>\n</rss>\n");
	fwrite(buf, sizeof(char), strlen(buf), fp);

	fclose(fp);
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
	int len;

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
			strcpy(cur_dir_name, c_file.name);
			//printf("fieldname: %s", c_file.name);
			check_dir(tmp_buf);
			continue;
		}

		len = strlen(c_file.name);
		if (len < 4) continue;
		if (_strnicmp(c_file.name + len - 4, ".txt", 4) == 0) {
			sprintf(tmp_buf, "%s\\%s", src_dir, c_file.name);
			convert_main(tmp_buf);
		}
		// printf("%s\n", c_file.name);
	}
	_findclose(hFile);
	return 0;

}

/* ************************************************** */
/*   テスト用の変換                                   */
/*    src_txt : 入力テキストファイルのフルパス        */
/* ************************************************** */
static int convert_main(char* src_txt) {

	char* src_buf;		// ソーステキストのデータの格納位置
	char txt_name[64];


	// ソースファイルをメモリに読み込む
	src_buf = read_file(src_txt);
	if (src_buf == NULL) {
		return -1;
	}

	// ソースファイルの名前を取り出しておく
	_splitpath(src_txt, NULL, NULL, txt_name, NULL);

	strcpy(field[total], cur_dir_name);
	strcpy(fname[total], txt_name);

	search_date(src_buf);

	printf("convert [%d][%s]\n", t_int[total-1], src_txt);

	free(src_buf);

	return 0;
}

/* ************************************************** */
/*  最終更新日時を得てグローバル変数に記録する        */
/*  src: ソース先頭                                   */
/* ************************************************** */
static void search_date(char* src) {

	int ret_cnt;
	char tmp_buf[256];
	unsigned long create;
	unsigned long modify;

	ret_cnt = 0;
	create = 0;
	modify = 0;

	read_total++;

	while (*src != 0) {

		if (*src == '\n') {
			ret_cnt++;
			if (ret_cnt > 20) {
				break;
			}
		}
		if (*src == '\\') {

			if (strncmp(src + 1, "create", 6) == 0) {
				src += 8;
				src = set_param(tmp_buf, src);
				create = int_date(tmp_buf);
				continue;
			}
			else if (strncmp(src + 1, "modify", 6) == 0) {
				src += 8;
				src = set_param(tmp_buf, src);
				modify = int_date(tmp_buf);
				break;
			}

		}
		src++;
	}

	if (create == 0 && modify == 0) {
		return;
	}

	if (modify > 0) {
		t_int[total] = modify;
	}
	else {
		t_int[total] = create;
	}
	total++;

}

/* ************************************************** */
/*   日付順にソートされた num 番目の記事を開いて     */
/*   各種文字列を埋める。             */
/* ************************************************** */
static int get_data(int num) {

	int idx;
	char buf[256];
	char* src_buf;

	title[0] = 0;
	category[0] = 0;
	desc[0] = 0;
	date[0] = 0;

	idx = index[num];

	// チャプターは新規記事として出さないようにする
	if (strncmp(fname[idx], "chapter", 7) == 0) {
		return -1;
	}
	// リンク文字列を作成
	sprintf(link, "https://eman-physics.net/%s/%s.html", field[idx], fname[idx]);

	// 該当記事を開く作業
	sprintf(buf, "c:\\physics\\site\\%s\\%s.txt", field[idx], fname[idx]);

	// 記事の txt ファイルをメモリに読み込む
	src_buf = read_file(buf);
	if (src_buf == NULL) {
		return -1;
	}
	search_article(src_buf);
	free(src_buf);

	return 0;
}

/* ************************************************** */
/*  記事原稿から必要な情報を抽出する                  */
/*  src: ソース先頭                                   */
/* ************************************************** */
static void search_article(char* src) {

	int ret_cnt;
	char tmp_buf[256];
	char alt[256];
	int i;
	char text[2048];
	int text_idx;
	int text_cnt;
	int rc;
	int f_modified;
	int f_text_start;
	int f_tex;
	int f_comment_out;

	ret_cnt = 0;
	f_modified = false;
	f_text_start = false;
	f_tex = false;
	f_comment_out = false;

	while (*src != 0) {

		if (*src == '\n') {
			ret_cnt++;
			if (ret_cnt > 40) {
				break;
			}
		}

		if (f_comment_out) { // 次の改行が来るまでは無視
			if (*src == '\n') {
				f_comment_out = false;
				src++;
				continue;
			}
			src++;
			continue;
		}

		if (f_tex) { // 次の</tex>が来るまでは無視
			if (*src == '<') {
				if (strncmp(src, "</tex>", 6) == 0) {
					f_tex = false;
					src += 6;
					continue;
				}
			}
			src++;
			continue;
		}


		if (f_text_start) {

			// 全角スペースは排除
			if (((unsigned char)*src == 0xe3) && ((unsigned char)*(src + 1) == 0x80) && ((unsigned char)*(src + 2) == 0x80)) {
				src += 3;
				continue;
			}

			if (((unsigned char)*src & 0xc0) == 0x80) {
				// UTF8の2バイト目以降の文字なのでそのままコピーしてスルー
				text[text_idx++] = *(src++);
				continue;
			}

			if (text_cnt >= 140) {
				// 引用文字数が140字を超えており、マルチバイト文字の途中でもないのでここで終了
				break;
			}

			if (((unsigned char)*src & 0x80) == 0x80) { // 何らかの全角文字の先頭である
				text[text_idx++] = *(src++);
				text_cnt++;
				continue;
			}

			// 改行は排除
			if (*src == '\n'){
				src++;
				if (*src == '%') {
					f_comment_out = true;
				}
				continue;
			}

			// 改行は排除
			if (*src == '\r') {
				src++;
				continue;
			}

			// 半角空白も排除
			if (*src == ' ') {
				src++;
				continue;
			}

			if (*src == '$') {
				// 終わりの $ までコピー
				src++;
				i = 0;
				while (*src != '$') {
					tmp_buf[i++] = *(src++);
				}
				src++;
				tmp_buf[i] = 0;
				make_alt_string(tmp_buf, alt);
				strcpy(text + text_idx, alt);
				text_idx += strlen(alt);
				text_cnt += u8len(alt);
			}

			if (*src == '<') {
				if (strncmp(src + 1, "tex", 3) == 0) {
					f_tex = true;
					strcpy(tmp_buf, u8"（数式）");
					strcpy(text + text_idx, tmp_buf);
					text_idx += strlen(tmp_buf);
					text_cnt += 4;
					continue;
				}

				if (strncmp(src, "<P>", 3) == 0) {
					src += 3;
					continue;
				}
				if (strncmp(src, "<p>", 3) == 0) {
					src += 3;
					continue;
				}
				if (strncmp(src, "<br>", 4) == 0) {
					src += 4;
					continue;
				}
				if (strncmp(src, "<BR>", 4) == 0) {
					src += 4;
					continue;
				}
				if (strncmp(src, "<a ", 3) == 0) {
					while (*(src++) != '>');
					continue;
				}
				if (strncmp(src, "<A ", 3) == 0) {
					while (*(src++) != '>');
					continue;
				}
				if (strncmp(src, "</a>", 4) == 0) {
					src += 4;
					continue;
				}
				if (strncmp(src, "</A>", 4) == 0) {
					src += 4;
					continue;
				}
				if (strncmp(src, "<center>", 8) == 0) {
					src += 8;
					continue;
				}
				if (strncmp(src, "<CENTER>", 8) == 0) {
					src += 8;
					continue;
				}
				if (strncmp(src, "</center>", 9) == 0) {
					src += 9;
					continue;
				}
				if (strncmp(src, "</CENTER>", 9) == 0) {
					src += 9;
					continue;
				}
				if (strncmp(src, "<iframe", 7) == 0) {
					src += 7;
					while (*(src++) != '>');
					continue;
				}
				if (strncmp(src, "</iframe>", 9) == 0) {
					src += 9;
					continue;
				}
				if (strncmp(src, "<img", 4) == 0) {
					src += 9;
					while (*(src++) != '>');
					strcpy(tmp_buf, u8"（図）");
					strcpy(text + text_idx, tmp_buf);
					text_idx += strlen(tmp_buf);
					text_cnt += 3;
					continue;
				}

			}

			if (*src == '\\') {
				if (strncmp(src + 1, "section", 7) == 0) {
					src += 8;
					while (*(src++) != '}');
					continue;
				}
				if (strncmp(src + 1, "image", 5) == 0) {
					src += 6;
					while (*(src++) != '}');
					strcpy(tmp_buf, u8"（図）");
					strcpy(text + text_idx, tmp_buf);
					text_idx += strlen(tmp_buf);
					text_cnt += 3;
					continue;
				}
				if (strncmp(src + 1, "red", 3) == 0) {
					src += 5;
					src = set_param(tmp_buf, src);
					strcpy(text + text_idx, tmp_buf);
					text_idx += strlen(tmp_buf);
					text_cnt += u8len(tmp_buf);
					continue;
				}
				if (strncmp(src + 1, "black", 5) == 0) {
					src += 7;
					src = set_param(tmp_buf, src);
					strcpy(text + text_idx, tmp_buf);
					text_idx += strlen(tmp_buf);
					text_cnt += u8len(tmp_buf);
					continue;
				}
				if (strncmp(src + 1, "note", 4) == 0) {
					src += 6;
					while (*(src++) != '}');
					strcpy(tmp_buf, u8"（注釈）");
					strcpy(text + text_idx, tmp_buf);
					text_idx += strlen(tmp_buf);
					text_cnt += 4;
					continue;
				}


			}

			// 半角文字である。
			text[text_idx++] = *(src++);
			text_cnt++;
			continue;

		}


		if (*src == '\\') {

			if (strncmp(src + 1, "create", 6) == 0) {
				src += 8;
				src = set_param(tmp_buf, src);
				rc = make_date_str(tmp_buf);
				continue;
			}
			else if (strncmp(src + 1, "modify", 6) == 0) {
				src += 8;
				src = set_param(tmp_buf, src);
				rc = make_date_str(tmp_buf);
				if (rc == 0) {
					f_modified = true;
				}
				continue;
			}
			else if (strncmp(src + 1, "field", 5) == 0) {
				src += 7;
				src = set_param(category, src);
				continue;
			}
			else if (strncmp(src + 1, "title", 5) == 0) {
				src += 7;
				src = set_param(tmp_buf, src);
				remove_tags(title, tmp_buf);
				continue;
			}
			else if (strncmp(src + 1, "section", 7) == 0) {
				src += 7;
				src = set_param(tmp_buf, src); // 別に使わないけど入れる
				f_text_start = true;
				text_idx = 0;
				text_cnt = 0;
				continue;
			}
		}
		src++;
	}

	text[text_idx] = 0;

	if (f_modified) {
		sprintf(desc, u8"【修正】%s……", text);
	} else {
		sprintf(desc, u8"【新規記事】%s……", text);
	}

}

/* ************************************************** */
/*   utf-8の文字数を数える     */
/* ************************************************** */
static int u8len(char* src){

	int cnt;

	cnt = 0;
	
	while (*src != 0) {
		if ((*src & 0xc0) != 0x80) { cnt++; }  // 
		src++;
	}
	return cnt;
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
/*   create や modify の時刻表記を整数化する          */
/*  日付順にソートするための一時的な記録　*/
/* ************************************************** */
static unsigned long int_date(char* src) {

	int year = 0;
	int month = 0;
	int day = 0;
	int hour = 0;
	int min = 0;
	int sec = 0;
	unsigned long num;

	sscanf(src, "%d/%d/%d %d:%d:%d", &year, &month, &day, &hour, &min, &sec);
	if (year == 0 || month == 0 || day == 0) {
		return 0;
	}
	//printf("%d/%d/%d %d:%d:%d\n", year, month, day, hour, min, sec);
	num = year - 2000;
	num = num * 100 + month;
	num = num * 100 + day;
	num = num * 100 + hour;
	num = num * 100 + min;

	return num;
}

/* ************************************************** */
/*   日付からRSS用の文字列を生成して */
/*  グローバル文字列に格納する          */
/* ************************************************** */
static int make_date_str(char* src) {

	int year = 0;
	int month = 0;
	int day = 0;
	int hour = 0;
	int min = 0;
	int sec = 0;

	struct tm time;
	time_t t;
	struct tm* local;

	sscanf(src, "%d/%d/%d %d:%d:%d", &year, &month, &day, &hour, &min, &sec);
	if (year == 0 || month == 0 || day == 0) {
		return -1;
	}

	time.tm_year = year - 1900;
	time.tm_mon = month-1;
	time.tm_mday = day;

	time.tm_hour = 0;
	time.tm_min = 0;
	time.tm_sec = 0;

	//printf("%d/%d/%d %d:%d:%d\n", year, month, day, hour, min, sec);
	t = mktime(&time);
	if (t == -1) {
		printf("時間の変換が失敗しました\n");
		return -1;
	}

	local = localtime(&t);
	//printf("week: %s\n", week_str[local->tm_wday]);

	sprintf(date, "%s, %d %s %d %d:%02d:%02d +0900", week_str[local->tm_wday], day, mon_str[month-1], year, hour, min, sec);

	return 0;
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
/*      TeX数式を翻訳して alt 文字列を生成            */
/* ************************************************** */
static void make_alt_string(char* equation, char* alt) {

	static char buf[4096];
	static char buf2[4096];
	int len;

	alt_alias(equation, buf2);
	alt_vec(buf2, buf);   // \Vec は外して中身だけを出す
	alt_index(buf, buf2);   // \Vec は外して中身だけを出す
	alt_sqrt(buf2, buf);
	alt_frac(buf, buf2);
	alt_braket(buf2, buf);
	alt_greek(buf, buf2);
	alt_space(buf2, alt);

	// 最後尾が空白だった場合、それを取り除く
	len = strlen(alt);
	if (alt[len - 1] == ' ') {
		alt[len - 1] = 0;
	}
}

/* ************************************************** */
/*  幾つかのコマンドを予め書き換え           */
/* ************************************************** */
static void alt_alias(char* src, char* dst) {

	char* begin;
	begin = src;

	while (*src != 0) {

		if (src - begin > 512) {
			printf(" error: alt_alias\n");
			exit(1);
		}

		if (*src == '\\') {

			// 添字
			if (strncmp(src + 1, "sup", 3) == 0) {
				src += 4;
				*(dst++) = '^';
				continue;
			}

			// 添字
			if (strncmp(src + 1, "sub", 3) == 0) {
				src += 4;
				*(dst++) = '_';
				continue;
			}

			// サイズ変更コマンドは無視
			if (strncmp(src + 1, "scriptstyle", 11) == 0) {
				src += 12;
				continue;
			}

			// サイズ変更コマンドは無視
			if (strncmp(src + 1, "scriptscriptstyle", 17) == 0) {
				src += 18;
				continue;
			}

			// サイズ変更コマンドは無視
			if (strncmp(src + 1, "displaystyle", 12) == 0) {
				src += 13;
				continue;
			}

			// サイズ変更コマンドは無視
			if (strncmp(src + 1, "textstyle", 9) == 0) {
				src += 10;
				continue;
			}

			// 微妙な空白も無視
			// しかし外したことでコマンドが誤解される可能性があるので空白を入れる
			// \alpha\,b → \alphab になってしまうような感じ
			if (*(src + 1) == ',') {
				src += 2;
				*(dst++) = ' ';
			}

			// 位置調整コマンドも無視
			if (*(src + 1) == '!') {
				src += 2;
				*(dst++) = ' ';
			}

			// 大きさ調整コマンドも無視
			if (strncmp(src + 1, "left", 4) == 0) {
				src += 5;
				continue;
			}

			// 大きさ調整コマンドも無視
			if (strncmp(src + 1, "right", 5) == 0) {
				if (strncmp(src + 1, "rightarrow", 10) != 0) { // right だけに反応してほしい \rightarrowは無視してほしい
					src += 6;
					continue;
				}
			}

		}

		*(dst++) = *(src++);
	}
	*dst = 0;
}

/* ************************************************** */
/* ベクトルは外して普通の文字として取り出す           */
/* ************************************************** */
static void alt_vec(char* src, char* dst) {

	char* begin;
	begin = src;

	while (*src != 0) {

		if (src - begin > 512) {
			printf(" error: alt_vec\n");
			exit(1);
		}

		if (*src == '\\') {
			if (strncmp(src + 1, "Vec", 3) == 0) {
				src += 5; // \Vec{ の次まで飛ばす
				while (*src != '}') { // 括弧の終わりまでコピー。2重括弧はないとしてよい
					*(dst++) = *(src++);
				}
				src++;
				*(dst++) = ' '; // 括弧を外したことでコマンドの解釈が壊れる可能性があるので仮の空白を入れておく
				continue;
			}

			if (strncmp(src + 1, "mathcal", 7) == 0) {
				src += 9; // \mathcal{ の次まで飛ばす
				while (*src != '}') { // 括弧の終わりまでコピー。2重括弧はないとしてよい
					*(dst++) = *(src++);
				}
				src++;
				*(dst++) = ' '; // 括弧を外したことでコマンドの解釈が壊れる可能性があるので仮の空白を入れておく
				continue;
			}

			if (strncmp(src + 1, "mathrm", 6) == 0) {
				src += 8; // \mathrm{ の次まで飛ばす
				while (*src != '}') { // 括弧の終わりまでコピー。2重括弧はないとしてよい
					*(dst++) = *(src++);
				}
				src++;
				*(dst++) = ' '; // 括弧を外したことでコマンドの解釈が壊れる可能性があるので仮の空白を入れておく
				continue;
			}

			if (strncmp(src + 1, "text", 4) == 0) {
				src += 6; // \mathrm{ の次まで飛ばす
				while (*src != '}') { // 括弧の終わりまでコピー。2重括弧はないとしてよい
					*(dst++) = *(src++);
				}
				src++;
				*(dst++) = ' '; // 括弧を外したことでコマンドの解釈が壊れる可能性があるので仮の空白を入れておく
				continue;
			}

		}

		*(dst++) = *(src++);
	}
	*dst = 0;
}

/* ************************************************** */
/* 　添え字の処理           */
/* ************************************************** */
static void alt_index(char* src, char* dst) {

	char* begin;
	begin = src;

	while (*src != 0) {

		if (src - begin > 512) {
			printf(" error: alt_index\n");
			exit(1);
		}

		if (*src == '_' || *src == '^') {
			if (*(src + 1) == '{') {
				*(dst++) = *(src++);
				*(dst++) = *(src++);
				continue;
			}

			// 空白を探す
			while (*src != ' ') {
				*(dst++) = *(src++);
			}
			// 空白が見付かった
			*src++;
			strcpy(dst, "\\ ");
			dst += 2;
			continue;
		}

		*(dst++) = *(src++);
	}
	*dst = 0;
}

/* ************************************************** */
/* ルートの書き換え                                   */
/* ************************************************** */
static void alt_sqrt(char* src, char* dst) {

	char buf[128];
	int idx;
	int brace;
	int len;

	char* begin;
	begin = src;

	while (*src != 0) {

		if (src - begin > 512) {
			printf(" error: alt_sqrt\n");
			exit(1);
		}

		if (*src == '\\') {
			if (strncmp(src + 1, "sqrt", 4) == 0) {
				src += 5;

				if (*src != '{') {
					continue;
				}

				src++; // \sqrt{ の次まで飛ばす

				// 括弧の終わりまで buf にコピー。2重括弧はあるかもしれない
				idx = 0;
				brace = 1;
				while (1) {
					if (*src == '}') {
						brace--;
						if (brace == 0) {
							src++;
							break;
						}
					}
					else if (*src == '{') {
						brace++;
					}
					buf[idx] = *src;
					idx++;
					src++;
				}
				buf[idx] = 0;

				if ((alt_count_variables(buf) > 1) && (alt_exist_paren(buf) == false)) {
					sprintf(dst, u8"√(%s)\\ ", buf);
					len = strlen(dst);
					dst += len;
				}
				else {
					sprintf(dst, u8"√%s\\ ", buf);
					len = strlen(dst);
					dst += len;
				}
				continue;
			}
		}

		*(dst++) = *(src++);
	}
	*dst = 0;
}

/* ************************************************** */
/* 分数部分を割算に書き換える作業 */
/* ************************************************** */
typedef enum {
	FRAC_NONE = 0,
	FRAC_FRAC,
	FRAC_DIF,
	FRAC_DDIF,
	FRAC_PDIF,
	FRAC_PDDIF,
	FRAC_PDDDIF,
	FRAC_PDIFLINE
} FRAC_TYPES;

static void alt_frac(char* src, char* dst) {

	int count;
	char buf[128];
	int n;
	int mode;

	char* begin;
	begin = src;

	count = 0;

	while (*src != 0) {

		if (src - begin > 512) {
			printf(" error: alt_frac\n");
			exit(1);
		}

		if (*src == '\\') {
			mode = 0;

			if (strncmp(src + 1, "diff", 4) == 0) {
				// 割り算と関係ないコマンドだが\difと間違えられるから真っ先に除外
				src += 5;
				strcpy(dst, "\\ d");
				dst += 3;
				continue;
			}
			else if (strncmp(src + 1, "frac", 4) == 0) {
				src += 5;
				mode = FRAC_FRAC;
			}
			else if (strncmp(src + 1, "dif", 3) == 0) {
				src += 4;
				mode = FRAC_DIF;
			}
			else if (strncmp(src + 1, "ddif", 4) == 0) {
				src += 5;
				mode = FRAC_DDIF;
			}
			else if (strncmp(src + 1, "pdifline", 8) == 0) {
				// pdif よりも上に記述しないと pdif と誤解されてしまう
				src += 9;
				mode = FRAC_PDIFLINE;
			}
			else if (strncmp(src + 1, "pdif", 4) == 0) {
				src += 5;
				mode = FRAC_PDIF;
			}
			else if (strncmp(src + 1, "pddif", 5) == 0) {
				src += 6;
				mode = FRAC_PDDIF;
			}
			else if (strncmp(src + 1, "pdddif", 6) == 0) {
				src += 7;
				mode = FRAC_PDDDIF;
			}

			if (mode != 0) {
				if (count > 0) {
					// 分数より前に何か文字があれば分数の直前に空白を入れる
					*(dst++) = '\\';
					*(dst++) = ' ';
				}
				n = alt_frac_sub(src, buf, mode);
				src += n;
				strcpy(dst, buf);
				dst += strlen(buf);
				count = 0; // 分数の直後は空白が入れてあるのですぐに分数が来る場合には空白を入れなくて良くなる
				continue;
			}
		}

		if (*src == '=') {
			// イコールの直後なら分数は詰めて書くのでカウンターはリセットされる
			*(dst++) = *(src++);
			count = 0;
			continue;
		}

		if (*src == ' ') {
			// 空白はカウントしない
			*(dst++) = *(src++);
			continue;
		}

		*(dst++) = *(src++);
		count++;
	}
	*dst = 0;
}

/* ************************************************** */
/* 割り算部分をどう書き替えるかを返す                 */
/* 戻り値 srcから数えて何バイト目が割り算の終点かを返す */
/*  src: \fracから始まる文字列を入れる(終端は不明）   */
/*  dst: どう書き替えたらいいかを返す                 */
/* ************************************************** */
static int alt_frac_sub(char* src, char* dst, int mode) {

	int brace_num;
	int idx;
	char buf1[128];
	char buf2[128];
	char bunshi[128];
	char bunbo[128];
	int size1;
	int size2;
	int start;

	// 最初の { を飛ばす
	idx = 1;
	//printf("in [%s]\n", src);
	brace_num = 1;
	while (1) {

		if (idx > 512) {
			printf("error: frac sub 1\n");
			exit(1);
		}

		if (*(src + idx) == '{') {
			brace_num++;
			idx++;
			continue;
		}

		if (*(src + idx) == '}') {
			brace_num--;
			if (brace_num == 0) {
				break;
			}
		}
		idx++;
	}
	//printf("out\n");

	// この段階で idx は } を指している
	// idx は指している文字より前にある文字数を表している
	size1 = idx - 1;  // これは分子部分の文字数
	strncpy(buf1, src + 1, size1);
	buf1[size1] = 0;

	idx += 2; // }{ の二文字を飛ばす
	start = idx;  // 分母の開始位置を記録

	brace_num = 1;
	while (1) {

		if (idx > 512) {
			printf("error: frac sub 2 \n");
			exit(1);
		}

		if (*(src + idx) == '{') {
			brace_num++;
			idx++;
			continue;
		}

		if (*(src + idx) == '}') {
			brace_num--;
			if (brace_num == 0) {
				break;
			}
		}
		idx++;
	}
	// この段階で idx は } を指している
	size2 = idx - start;
	strncpy(buf2, src + start, size2);
	buf2[size2] = 0;

	//printf("Debug: [%s] [%s]\n", buf1, buf2);

	if (alt_count_variables(buf1) > 1 && alt_exist_paren(buf1) == false) {
		sprintf(bunshi, "(%s)", buf1);
	}
	else {
		strcpy(bunshi, buf1);
	}

	if (alt_count_variables(buf2) > 1 && alt_exist_paren(buf2) == false) {
		sprintf(bunbo, "(%s)", buf2);
	}
	else {
		strcpy(bunbo, buf2);
	}

	switch (mode) {
	case FRAC_FRAC:
		sprintf(dst, "%s/%s\\ ", bunshi, bunbo);
		break;
	case FRAC_DIF:
		sprintf(dst, "d%s/d%s\\ ", bunshi, bunbo);
		break;
	case FRAC_DDIF:
		sprintf(dst, "d^2%s/d%s^2\\ ", bunshi, bunbo);
		break;
	case FRAC_PDIF:
		sprintf(dst, "\\partial %s/\\partial %s\\ ", bunshi, bunbo);
		break;
	case FRAC_PDDIF:
		sprintf(dst, "\\partial^2 %s/\\partial %s^2\\ ", bunshi, bunbo);
		break;
	case FRAC_PDDDIF:
		sprintf(dst, "\\partial^3 %s/\\partial %s^3\\ ", bunshi, bunbo);
		break;
	case FRAC_PDIFLINE:
		sprintf(dst, "\\partial %s/\\partial %s\\ ", bunshi, bunbo);
		break;
	default:
		sprintf(dst, "%s/%s\\ ", bunshi, bunbo);
		break;
	}


	/*
sprintf(dst, "%s/%s\\ ", buf1, buf2);

printf("Debug: [%s] [%s] [%s]\n", buf1, buf2, dst);

char buf3[128];
strncpy(buf3, src, idx + 1);
buf3[idx + 1] = 0;
printf("Debug2: [%s] \n", buf3);
*/

	return idx + 1; // 最後の } も含めて文字数を返すので 1 を足す
}

/* ************************************************** */
/* src文字列に含まれる数式の文字数を数える            */
/* 　他のコマンド解析の下請けなので                   */
/*      ややこしいコマンドは含まれていないと仮定      */
/* ************************************************** */
static int alt_count_variables(char* src) {

	int cnt;
	int brace;

	char* begin;
	begin = src;

	cnt = 0;

	while (*src != 0) {

		if (src - begin > 512) {
			printf(" error: alt_variables\n");
			exit(1);
		}

		if (*src == '\\') {

			if (*(src + 1) == ' ') { //　空白はノーカウント
				src += 2;
				continue;
			}

			// その他のギリシャ文字などのコマンド
			cnt++;
			src++;
			while ((*src >= 0x41 && *src <= 0x5a) || (*src >= 0x61 && *src <= 0x7a)) {
				src++;
			}
			continue;
		}

		if (*src == '^' || *src == '_') { // 添え字はノーカウント

			if (*(src + 1) != '{') {
				src += 2;
				continue;
			}

			src += 2;
			brace = 1;
			while (1) {
				if (*src == '}') {
					brace--;
					if (brace == 0) {
						break;
					}
				}
				else if (*src == '{') {
					brace++;
				}

				src++;
			}
			src++;
			continue;
		}

		// ただの空白もノーカウント
		if (*src == ' ') {
			src++;
			continue;
		}

		src++;
		cnt++; // どれでもない何らかの文字らしいのでカウント
	}

	return cnt;
}

/* ************************************************** */
/* 一番外側が括弧で覆われているかどうかを判別         */
/* ************************************************** */
static int alt_exist_paren(char* src) {

	char* begin;
	begin = src;

	while (*src != 0) {

		if (src - begin > 512) {
			printf(" error: alt_exist_paren\n");
			exit(1);
		}

		if (*src == ' ') {
			src++;
			continue;
		}

		if (*src == '(') return true;

		return false;
	}
	return false;
}

/* ************************************************** */
/* ブラ・ケットの書き換え                                   */
/* ************************************************** */
static void alt_braket(char* src, char* dst) {

	char buf[128];
	int idx;
	int brace;
	int len;
	int mode;

	char* begin;
	begin = src;

	while (*src != 0) {

		if (src - begin > 512) {
			printf(" error: alt_braket\n");
			exit(1);
		}

		if (*src == '\\') {
			mode = 0;
			if (strncmp(src + 1, "bra", 3) == 0) {
				mode = 1;
			}
			else if (strncmp(src + 1, "ket", 3) == 0) {
				mode = 2;
			}

			if (mode > 0) {
				src += 5; // \bra{ の次まで飛ばす

				// 括弧の終わりまで buf にコピー。2重括弧はあるかもしれない
				idx = 0;
				brace = 1;
				while (1) {
					if (*src == '}') {
						brace--;
						if (brace == 0) {
							break;
						}
					}
					else if (*src == '{') {
						brace++;
					}
					buf[idx] = *src;
					idx++;
					src++;
				}
				buf[idx] = 0;

				if (mode == 1) {
					sprintf(dst, u8"<%s| ", buf);
				}
				else {
					sprintf(dst, u8"|%s> ", buf);
				}
				len = strlen(dst);
				dst += len;

				continue;
			}
		}

		*(dst++) = *(src++);
	}
	*dst = 0;
}

/* ************************************************** */
/* ギリシャ文字や記号を置き換える作業 */
/* ************************************************** */
static void alt_greek(char* src, char* dst) {

	int i;
	int n;
	int f_changed;

	const char cmd[68][16] = { "alpha","beta","gamma","delta","epsilon","varepsilon","zeta","eta","theta","vartheta",  // 10
			"iota", "kappa","lambda","mu","nu","xi","pi","rho","sigma","tau","upsilon","phi","varphi","chi","psi","omega", // 26
			"Gamma", "Delta","Theta","Lambda","Xi","Pi","Sigma","Upsilon","Phi","Psi","Omega", // 37
			"sum", "int", "prod","hbar","cdots","cdot","times","neq","dagger",
			"Rot", "Div", "Grad", "sin", "cos", "tan","exp","log", // 54
			"diff", "kinji", "partial", "lt","leqq", "geqq","ast","sim","equiv","rightarrow","infty","langle","rangle","pm" };

	const char greek[68][8] = { u8"α",u8"β", u8"γ", u8"δ", u8"ε", u8"ε", u8"ζ", u8"η", u8"θ", u8"θ",
			u8"ι",u8"κ", u8"λ", u8"μ", u8"ν", u8"ξ", u8"π", u8"ρ", u8"σ", u8"τ",
			u8"υ",u8"φ", u8"φ", u8"χ", u8"ψ", u8"ω",
			u8"Γ",u8"Δ", u8"Θ", u8"Λ", u8"Ξ", u8"Π", u8"Σ", u8"Υ", u8"Φ", u8"Ψ",u8"Ω",
			u8"Σ", u8"∫",u8"Π",u8"ℏ",u8"…",u8"・",u8"×",u8"≠",u8"†",
			u8"rot\\ ",u8"div\\ ",u8"grad\\ ",u8"sin",u8"cos",u8"tan",u8"exp",u8"log",
			u8"d", u8"≒", u8"∂", u8"&lt;", u8"≦", u8"≧",u8"*",u8"～",u8"≡",u8"→",u8"∞",u8"&lt;",u8"&gt;",u8"±" };

	while (*src != 0) {

		if (*src == '\\') {
			f_changed = false;
			for (i = 0; i < 68; i++) {
				n = strlen(cmd[i]);
				if (strncmp(src + 1, cmd[i], n) == 0) {
					src += (n + 1);
					strcpy(dst, greek[i]);
					dst += strlen(greek[i]);
					f_changed = true;
					break;
				}
			}
			if (f_changed) continue;
		}

		*(dst++) = *(src++);
	}

	*dst = 0;
}

/* ************************************************** */
/* 仕上げとして不要な空白を取り除く作業 */
/* ************************************************** */
static void alt_space(char* src, char* dst) {

	int f_first;
	int f_after_space;

	char* begin;
	begin = src;

	f_first = true;  // 最初の文字が来るまでは true
	f_after_space = false; // 空白の直後は true

	while (*src != 0) {

		if (src - begin > 512) {
			printf(" error: alt_space\n");
			exit(1);
		}

		if (*src == '\\') {

			// 微妙な空白は排除
			if (*(src + 1) == ',') {
				src += 2;
				continue;
			}

			// 明白な空白指定は残す
			if (*(src + 1) == ' ') {
				src += 2;
				if (f_first == false && f_after_space == false) {
					*(dst++) = ' ';
					f_after_space = true;
				}
				continue;
			}
		}
		else if (*src == ' ') { // ただの空白は排除
			src++;
			continue;
		}
		else if (*src == '<') { // 書き換えが必要
			src++;
			strcpy(dst, "&lt;");
			dst += 4;
			f_first = false;
			f_after_space = false;
			continue;
		}
		else if (*src == '>') { // 書き換えが必要
			src++;
			strcpy(dst, "&gt;");
			dst += 4;
			f_first = false;
			f_after_space = false;
			continue;
		}


		*(dst++) = *(src++);
		f_first = false;
		f_after_space = false;

	}

	*dst = 0;

}
