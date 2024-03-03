// svg_changer.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <io.h>
#include <string.h>
#include <time.h>

//#include <fcntl.h>
#include <windows.h>
//#include <process.h>


#define SRC_FOLDER			"c:\\physics\\converted\\site"
#define DST_FOLDER			"c:\\physics\\svg_ver"
#define WORK_FOLDER			"c:\\physics\\work"
#define TEX_HEADER_FILE		"c:\\physics\\header.tex"

#define INLINE_DATA_MAX 200
#define DISPLAY_DATA_MAX 200

static char* html_buf;
static char* tex_header_buf;
static char unique[32];
static char field[32];
static char title[32];
static int inline_num;     // インライン数式の数
static int display_num;    // ディスプレイ数式の数

static int f_make_img;     // 画像生成をするかしないか

static char inline_str[INLINE_DATA_MAX][64];

static float inline_x[INLINE_DATA_MAX];
static float inline_y[INLINE_DATA_MAX];

static float display_x[DISPLAY_DATA_MAX];
static float display_y[DISPLAY_DATA_MAX];

static int check_equation(char* src);
static void make_unique_number(void);
static int seek_unique_number(char* field, char* title);
static void make_folders(int mode);
static int exist_dir(char* dir);
static char* read_file(char* fname);
static int write_html(char* file, char* html_buf);
static void convert_main(void);
static void convert_sub(char* src, char* dst);
static int analyze_inline_equation(char* src, char* dst);
static int analyze_display_equation(char* src, char* dst);
static int check_equation_alignment(char* equation);
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
static int make_dvi(int f_inline, int num, char* equ_text);
static void replace_gvy(char* src, char* dst);
static void make_svg(void);
static void read_svg(void);
static int existFile(const char* path);
static void analyze_svg(char* fname, int f_inline, int num);
static void make_css(void);



int main(int argc, char* argv[]) {

	int rc;					// 結果
	char buf[256];			// 汎用

	//printf("SVG付きHTMLコンバータ for EMANの物理学\n\n");

	if (argc < 3) {
		printf("第1引数に分野名、第2引数に記事のファイル名（拡張子なし）を入れてください\n");
		return -1;
	}

	if (argc > 3) { // 第3引数に何か指定（「noimg」など）があれば argv==4 である
		f_make_img = false;
	} else {
		f_make_img = true;
	}

	// 引数は今後も使うので分かりやすい名前のグローバル変数へ入れる
	strcpy(field, argv[1]);
	strcpy(title, argv[2]);

	// 記事ファイルを読み出す（存在確認を兼ねる）
	sprintf(buf, "%s\\%s\\%s.html", SRC_FOLDER, field, title);
	html_buf = read_file(buf);
	if (html_buf == NULL) {
		printf("記事が開けません(%s)\n", buf);
		return -1;
	}

	// 記事中に変換すべき数式があるかどうかを確認
	rc = check_equation(html_buf);
	if (rc < 0) {
		printf("記事中に数式がありません。変換せずにコピーします\n");

		// 結果格納フォルダを作る
		make_folders(3);

		// コピーを行う
		sprintf(buf, "%s\\%s\\%s.html", DST_FOLDER, field, title);
		write_html(buf, html_buf);
		free(html_buf);
		return 0;
	}

	// TeX用ヘッダを読み込む
	sprintf(buf, "%s", TEX_HEADER_FILE);
	tex_header_buf = read_file(buf);
	if (tex_header_buf == NULL) {
		printf("TeX用ヘッダが開けません(%s)\n", buf);
		free(html_buf);
		return -1;
	}

	//printf("%d\n", f_make_img);
	if (f_make_img) {
		// 日付からユニークフォルダ名を作成する
		make_unique_number();
	} else {
		// 既にある画像フォルダの中の最新のものに合わせる
		rc = seek_unique_number(field, title);
		if (rc < 0) {
			// 見付からなかったので日付から作成する
			make_unique_number();
		}
	}

	//printf("test[%s]\n", unique);
	//return 0;

	// 作業用フォルダを作る
	make_folders(1);

	// 結果格納フォルダを作る
	make_folders(2);

	// 変換作業を行う。
	convert_main();

	// 作業終了
	free(html_buf);
	free(tex_header_buf);
	return 0;
}

/* ******************************************************* */
/*  数式が一つでもあるかどうかを確認　　　　               */
/* ******************************************************* */
int check_equation(char* src) {

	while (*src != 0) {
		
		if (*(src++) == '\\') {
			if (*src == '(' || *src == '[') return 0;
			src ++;
		}
	}

	return -1;
}
/* ******************************************************* */
/*  現在時刻からユニークナンバー文字列を作成               */
/* ******************************************************* */
void make_unique_number( void ) {

	time_t now;
	struct tm* tm;
	int year;

	now = time(NULL);
	tm = localtime(&now);

	year = tm->tm_year - 100;  // tm_year は1900年からの年数なので、100を引けば例えば今年は 21 になる。

	sprintf(unique, "%02d%02d%02d%02d%02d", year, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min);
}

/* ******************************************************* */
/*  フォルダ内を調べて最新の画像の入ったフォルダの         */
/*    ユニークナンバー文字列を探して設定                   */
/* ******************************************************* */
int seek_unique_number(char *field, char *title) {

	long   hFile;
	struct _finddata_t c_file;
	char target[64];
	int total;
	char kouho[32];
	int kouho_num;

	sprintf(target, "%s\\%s\\%s\\*", DST_FOLDER, field, title);
	hFile = _findfirst(target, &c_file);
	if (hFile < 0) {
		printf("フォルダ[%s]内に該当ファイル無し\n", target);
		return -1;
	}

	//printf("%s\n", c_file.name);

	total = 0;
	kouho_num = 0;
	while (_findnext(hFile, &c_file) == 0) {

		if (c_file.name[0] == '.') continue;

		if (c_file.attrib & _A_SUBDIR) {
			/* フォルダを発見 */
			if (strncmp("eq", c_file.name, 2) == 0) {
				total++;
				if (atoi(c_file.name+2) > kouho_num) {
					strcpy(kouho, c_file.name + 2);
					kouho_num = atoi(kouho);
				}
			}
			continue;
		}
	}
	_findclose(hFile);

	if (total == 0) return -1;

	strcpy(unique, kouho);
	return 0;

}

/* ******************************************************* */
/*  新しくフォルダを作る                                   */
/*  mode 1: 作業用フォルダを作る                           */
/*  mode 2: 結果用フォルダを作る                           */
/*  mode 3: 結果用フォルダ（数式無し）                     */
/* ******************************************************* */
static void make_folders(int mode) {

	char top[256];
	char buf[256];
	int rc;

	if (mode == 1) {
		sprintf(top, "%s", WORK_FOLDER);
	} else {
		sprintf(top, "%s", DST_FOLDER);
	}

	if (!exist_dir(top)) {
		rc = _mkdir(top);
		if (rc < 0) {
			printf("フォルダ作成に失敗しました。[%s]\n", top);
			return;
		}
		printf("フォルダを新規作成しました。[%s]\n", top);
	}

	// 分野名フォルダを作る
	sprintf(buf, "%s\\%s", top, field);

	if (!exist_dir(buf)) {
		rc = _mkdir(buf);
		if (rc < 0) {
			printf("フォルダ作成に失敗しました。[%s]\n", buf);
			return;
		}
		printf("フォルダを新規作成しました。[%s]\n", buf);
	}

	if (mode == 3) return;

	// 記事タイトルフォルダを作る
	sprintf(buf, "%s\\%s\\%s", top, field, title);

	if (!exist_dir(buf)) {
		rc = _mkdir(buf);
		if (rc < 0) {
			printf("フォルダ作成に失敗しました。[%s]\n", buf);
			return;
		}
		printf("フォルダを新規作成しました。[%s]\n", buf);
	}

	// ユニークフォルダを作る
	sprintf(buf, "%s\\%s\\%s\\eq%s", top, field, title, unique);

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

/* ************************************************** */
/*      変換作業                                  */
/* ************************************************** */
static void convert_main( void ) {

	char buf[256];
	char* result_buf;
	int buf_size;

	// 変換結果を入れるバッファとしてソースファイルの3倍程度を確保
	buf_size = 6 * strlen(html_buf);

	result_buf = (char*)malloc(buf_size);
	if (result_buf == NULL) {
		printf("変換結果用バッファを確保できませんでした。\n");
		return;
	}

	inline_num = 0;
	display_num = 0;

	// 変換作業の下請け
	convert_sub(html_buf, result_buf);

	if (f_make_img) {
		// dvi ファイルが全て仕上がったのを確認して SVG 画像に変換する
		Sleep(5000); // どうせすぐにはdviファイルは揃わないのでしばらく待つ
		printf("dvi check start\n");
		make_svg();
		Sleep(1000);

		// 出来上がったSVG画像を検査する
		printf("svg check start\n");
		read_svg();

		// 集めたデータからCSSファイルを作成する
		make_css();
	}

	// 結果の書き込み
	// これを最後にしておかないと、画像生成でエラーが出ていてもHTMLが先に完成してしまい
	// 正しく終わったかどうかの判断が付きにくい
	sprintf(buf, "%s\\%s\\%s.html", DST_FOLDER, field, title);
	printf("Write HTML [%s]\n", buf);
	write_html(buf, result_buf);

	free(result_buf);

}

/* ************************************************** */
/*      変換作業の下請け                              */
/* ************************************************** */
static void convert_sub(char* src, char* dst) {

	int f_link_inserted;
	int num;
	static char buf[2048]; // 汎用

	f_link_inserted = false;

	while (*src != 0) {

		if (f_link_inserted == false) {
			if (*src == '<') {
				if (strncmp(src + 1, "link", 4) == 0) {

					// 次の改行まではそのままコピー
					while (*src != '\n') *(dst++) = *(src++);

					sprintf(buf, "\n<link rel=\"stylesheet\" media=\"screen and (max-width:480px)\" href=\"./%s/eq%s/sp.css\" type=\"text/css\" />", title, unique);
					strcpy(dst, buf);
					dst += strlen(buf);

					sprintf(buf, "\n<link rel=\"stylesheet\" media=\"screen and (min-width:481px)\" href=\"./%s/eq%s/pc.css\" type=\"text/css\" />", title, unique);
					strcpy(dst, buf);
					dst += strlen(buf);

					f_link_inserted = true;
					continue;
				}
			}
		}

		if (*src == '\\') {
			if (*(src + 1) == '(') {
				num = analyze_inline_equation( src, buf);
				//printf("Debug num: %d\n", num);
				src += num;
				strcpy(dst, buf);
				dst += strlen(buf);
				continue;
			}
			
			if (*(src + 1) == '[') {
				num = analyze_display_equation(src, buf);
				//printf("Debug num: %d\n", num);
				src += num;
				strcpy(dst, buf);
				dst += strlen(buf);
				continue;
			}
		}

		*(dst++) = *(src++);
	}

	*dst = 0;

	//printf("convert sub Out\n");
}

/* ************************************************** */
/*      インライン数式の解析部                        */
/*  src: 「\(」 から始まる文字列                      */
/*  num: 「\)」で終わるところまでの長さを返す         */
/*  buf:  代わりに書き込むべき文字列を返す            */
/* ************************************************** */
static int analyze_inline_equation(char* src, char* dst) {

	int num;
	char* start;
	static char equation[256];
	static char alt[128];
	static char buf[256]; // 汎用
	int align_mode;
	int i;
	int f_new;

	f_new = false;

	start = src;

	// 数式の終わりを探す
	// 最初の二文字は調べる必要なし
	src += 2;
	num = 2;

	while (1) {
		if (*src == '\\'){
			if (*(src + 1) == ')') {
				num += 2;
				break;
			}
			// ￥マークの次の文字は見逃さないといけない. ここでの対象は「￥￥」が来る場合だけだけど
			src+=2;
			num+=2;
			continue;
		}
		src++;
		num++;
	}

	// 数式部分だけを equation として取り出し（srcをいじってはいけないから）
	strncpy(equation, start, num);
	equation[num] = 0;

	// すでにある式と同じかどうかのチェック
	// 面倒なので先頭の「\(」は外さない
	for (i = 0; i < inline_num; i++) {
		if (strcmp(equation, inline_str[i]) == 0) {
			break;
		}
	}
	if (i == inline_num) { // 同じものがなかった
		f_new = true;
		strcpy(inline_str[i], equation); // 新規登録
		inline_num++;
		make_dvi(1, i+1, equation);
	}

	// 文字の表示位置をチェック
	align_mode = check_equation_alignment(equation);
	//printf("mode: %d\n", align_mode);

	// alt文字列の生成
	if (f_new) printf("equ: [%s]\n", equation);
	make_alt_string(equation, alt);
	if( f_new ) printf("alt: [%s]\n\n", alt);
//	strcpy(alt, u8"数式");

	if (align_mode == 0) {
		sprintf(dst, "<img src=\"./%s/eq%s/i%03d.svg\" class=\"eq_i%d\" alt=\"%s\">", title, unique, i+1, i+1, alt);
	}
	else {
		sprintf(dst, "<img src=\"./%s/eq%s/i%03d.svg\" class=\"eq_i%d eq_v%d\" alt=\"%s\">", title, unique, i+1, i+1, align_mode, alt);
	}

	return num;
}

/* ************************************************** */
/*      ディスプレイ数式の解析部                        */
/*  src: 「\[」 から始まる文字列                      */
/*  num: 「\]」で終わるところまでの長さを返す         */
/*  buf:  代わりに書き込むべき文字列を返す            */
/* ************************************************** */
static int analyze_display_equation(char* src, char* dst) {

	int num;
	char* start;
	static char equation[8192];
	static char buf[256]; // 汎用

	start = src;

	// 最初の二文字は調べる必要なし
	src += 2;
	num = 2;

	while (1) {
		if (*src == '\\') {
			if (*(src + 1) == ']') {
				num += 2;
				break;
			}
			// ￥マークの次の文字は見逃さないといけない. ここでの対象は「￥￥」が来る場合だけだけど
			src += 2;
			num += 2;
			continue;
		}
		src++;
		num++;
	}

	// 数式部分だけを equation として取り出し（srcをいじってはいけないから）
	strncpy(equation, start, num);
	equation[num-2] = 0;  // 末尾に付いている "\\]" の2文字を外している

	display_num++;
	make_dvi(0, display_num, equation+3); /* 先頭に付いている "\\[\n" の3文字を外している */
	sprintf(dst, u8"<img src=\"./%s/eq%s/d%03d.svg\" class=\"eq_d%d\" alt=\"数式\">", title, unique, display_num, display_num);

	return num;
}

/* ************************************************** */
/*      TeX数式から、美しく見える上下調整を分類       */
/*  0: 移動の必要なし                                 */
/*  1: 少し移動   （普通の文字など）                  */
/*  2: 大きく移動（分数やfgjpqyQβζηξρφχψ)     */
/*  3: 微妙に移動（CやG）     */
/*  4: 少し多めに移動（z）     */
/* ************************************************** */
static int check_equation_alignment(char* equation) {

	int f_short;
	int f_middle;
	int brace_num;

	f_short = false;
	f_middle = false;

	//printf("debug: [%s]\n", equation);
	equation += 2;

	while (*equation != 0) {

		if (*equation == '\\') {

			if (strncmp(equation + 1, "diff", 4) == 0) { // この後の dif に該当して誤判定されてしまうので逃がす
				equation += 5;
				continue;
			}

			if (strncmp(equation + 1, "frac", 4) == 0) {
				return 2;
			}

			if (strncmp(equation + 1, "dif", 3) == 0) {
				return 2;
			}

			if (strncmp(equation + 1, "ddif", 4) == 0) {
				return 2;
			}

			if (strncmp(equation + 1, "pdifline", 8) == 0) {
				return 2;
			}

			if (strncmp(equation + 1, "pdif", 4) == 0) {
				return 2;
			}

			if (strncmp(equation + 1, "pddif", 5) == 0) {
				return 2;
			}

			if (strncmp(equation + 1, "bra", 3) == 0) {
				return 2;
			}

			if (strncmp(equation + 1, "ket", 3) == 0) {
				return 2;
			}

			if (strncmp(equation + 1, "langle", 6) == 0) {
				return 2;
			}

			if (strncmp(equation + 1, "rangle", 6) == 0) {
				return 2;
			}

			if (strncmp(equation + 1, "sub", 3) == 0) {
				f_short = true;
				equation += 4;
				continue;
			}

			if (strncmp(equation + 1, "sum", 3) == 0) {
				return 2;
			}

			if (strncmp(equation + 1, "beta", 4) == 0) {
				return 2;
			}

			if (strncmp(equation + 1, "gamma", 5) == 0) {
				return 2;
			}

			if (strncmp(equation + 1, "zeta", 4) == 0) {
				return 2;
			}

			if (strncmp(equation + 1, "eta", 3) == 0) {
				return 2;
			}

			if (strncmp(equation + 1, "mu", 2) == 0) {
				return 2;
			}

			if (strncmp(equation + 1, "xi", 2) == 0) {
				return 2;
			}

			if (strncmp(equation + 1, "rho", 3) == 0) {
				return 2;
			}

			if (strncmp(equation + 1, "phi", 3) == 0) {
				return 2;
			}

			if (strncmp(equation + 1, "varphi", 6) == 0) {
				return 2;
			}

			if (strncmp(equation + 1, "chi", 3) == 0) {
				return 2;
			}

			if (strncmp(equation + 1, "psi", 3) == 0) {
				return 2;
			}

			if (*(equation + 1) == ' ') { // 空白コマンドは無視
				equation += 2;
				continue;
			}

			if (*(equation + 1) == ',') { // 空白コマンドは無視
				equation += 2;
				continue;
			}

			if (*(equation + 1) == '!') { // 空白コマンドは無視
				equation += 2;
				continue;
			}

			if (*(equation + 1) == ')') { // 数式の終わり
				equation += 2;
				continue;
			}

			// ￥コマンドの終わりまで飛ばす
			equation++;
			while (1) {
				if (*equation == ' ') break; // 空白
				if (*equation == '\\') break; // 別コマンドの始まり
				if (*equation == '^') break;
				if (*equation == '_') break;
				if (*equation == '{') break;
				if (*equation == '(') break;
				if (*equation == ')') break;
				equation++;
			}
			continue;
		}

		// 指数部分は無視する
		if (*equation == '^') {

			// 指数の直後のコマンドが終わるところまで飛ばす
			if (*(equation+1) == '\\') {
				equation++;
				while (1) {
					equation++;
					if (*equation == ' ') break;  // 区切りのスペース
					if (*equation == '\\') break; // 次のコマンド
				}
				continue;
			}

			// 指数の直後の括弧を抜けるところまで飛ばす
			if (*(equation+1) == '{') {
				equation++;
				brace_num = 1;
				while (1) {
					equation++;
					if (*equation == '}') {
						brace_num--;
						if (brace_num == 0) break;
					}
					if (*equation == '{') {
						brace_num++;
					}
				}
				equation++;
				continue;
			}

			// 指数の直後の一文字だけ飛ばす
			equation += 2;
			continue;
		}

		if (*equation == 'f'
			|| *equation == 'g'
			|| *equation == 'j'
			|| *equation == 'p'
			|| *equation == 'q'
			|| *equation == 'y'
			|| *equation == 'Q') {
			return 2;
		}

		if (*equation == 'C' || *equation == 'G') {
			f_short = true;
			equation++;
			continue;
		}

		if (*equation == 'z') {
			f_middle = true;
			equation++;
			continue;
		}

		// 下付きの添字がある場合、とりあえず少し下へ
		if (*equation == '_') {
			f_middle = true;
			equation++;
			continue;
		}

		// コンマ、スラッシュ、括弧、絶対値などがあるときは少し下へ
		if (*equation == ',') {
			f_middle = true;
			equation++;
			continue;
		}

		if (*equation == '/') {
			return 2;
			//f_middle = true;
			//equation++;
			//continue;
		}

		if (*equation == '(') {
			return 2;
			//f_middle = true;
			//equation++;
			//continue;
		}

		if (*equation == ')') {
			return 2;
			//f_middle = true;
			//equation++;
			//continue;
		}

		if (*equation == '|') {
			return 2;
			//f_middle = true;
			//equation++;
			//continue;
		}

		equation++;
	}

	if (f_middle) return 4;
	if (f_short) return 3;

	return 1;
}

/* ************************************************** */
/*      TeX数式を翻訳して alt 文字列を生成            */
/* ************************************************** */
static void make_alt_string(char* equation, char* alt) {

	static char buf[4096];
	static char buf2[4096];
	int len;

	/* 空白やコマンドを取り除く作業*/
	strcpy(buf, equation + 2); // 最初の \( は飛ばす
	len = strlen(buf);
	buf[len - 2] = 0; // 最後の \) も外す

	alt_alias(buf, buf2);
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

				src ++; // \sqrt{ の次まで飛ばす
				
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

				if ( (alt_count_variables(buf) > 1) && (alt_exist_paren(buf) == false) ) {
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

			if(mode != 0){
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

	if (alt_count_variables(buf1) > 1 && alt_exist_paren(buf1)==false) {
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

	return idx+1; // 最後の } も含めて文字数を返すので 1 を足す
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

			if( mode > 0){
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
			"diff", "kinji", "partial", "lt","leqq", "geqq","ast","sim","equiv","rightarrow","infty","langle","rangle","pm"};

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

/* ************************************************** */
/*      DVIファイル作成                               */
/* ************************************************** */
static int make_dvi(int f_inline, int num, char* equ_text) {

	static char tmp[256];
	static char fname[256];
	static char outdir[256];
	static char dvi_name[256];
	static char svg_name[256];
	static char equation[8192];

	static char cmd[256];
	FILE* fp;

	if (f_make_img == false) {
		return 0;
	}

	replace_gvy(equ_text, equation);

	sprintf(outdir, "%s\\%s\\%s\\eq%s", WORK_FOLDER, field, title, unique);

	if (f_inline)  {
		sprintf(fname, "%s\\%s\\%s\\eq%s\\i%03d.tex", WORK_FOLDER, field, title, unique, num);
		//sprintf(dvi_name, "%s\\%s\\%s\\eq%s\\i%03d.dvi", WORK_FOLDER, field, title, unique, num);
		//sprintf(svg_name, "%s\\%s\\%s\\eq%s\\i%03d.svg",  DST_FOLDER, field, title, unique, num);
	} else {
		sprintf(fname,    "%s\\%s\\%s\\eq%s\\d%03d.tex", WORK_FOLDER, field, title, unique, num);
		//sprintf(dvi_name, "%s\\%s\\%s\\eq%s\\d%03d.dvi", WORK_FOLDER, field, title, unique, num);
		//sprintf(svg_name, "%s\\%s\\%s\\eq%s\\d%03d.svg",  DST_FOLDER, field, title, unique, num);
	}

	fp = fopen(fname, "wb");
	if (fp == NULL) {
		printf("TeXソースファイルの作成に失敗しました。\n");
		return -1;
	}
	fwrite(tex_header_buf, sizeof(char), strlen(tex_header_buf), fp);
	fwrite(equation, sizeof(char), strlen(equation), fp);
	strcpy(tmp, "\r\n\r\n\\end{document}\r\n");
	fwrite(tmp, sizeof(char), strlen(tmp), fp);
	fclose(fp);

	sprintf(cmd, "start platex -output-directory=%s %s", outdir, fname);
	//printf("COMMAND: %s\n", cmd);
	system(cmd);
	Sleep(800);
/*
	sprintf(cmd, "dvisvgm -n --exact --fontmap=kanjix.map -o %s %s", svg_name, dvi_name);
	printf("COMMAND: %s\n", cmd);
	system(cmd);

	// 作ったsvgファイルの中身を調べてサイズを記録する
	analyze_svg(svg_name, f_inline, num);
*/
	return 0;
}

/* ************************************************** */
/*      TeX数式の改変                                 */
/* 数式中の g v y をそれぞれ                          */
/*        \varg \varv \vary に置き変える              */
/* ************************************************** */
static void replace_gvy(char* src, char* dst) {

	while (*src != 0) {

		// コマンドだけは変化させてしまわぬように避ける
		if (*src == '\\') {
			*dst++ = *src++;

			// \begin と \end はその後の{}内も変更してはいけない
			if (strncmp(src, "begin", 5) == 0) {
				while (*src != '}') {
					*dst++ = *src++;
				}
				continue;
			}

			if (strncmp(src, "end", 3) == 0) {
				while (*src != '}') {
					*dst++ = *src++;
				}
				continue;
			}

			if (strncmp(src, "text", 4) == 0) {
				while (*src != '}') {
					*dst++ = *src++;
				}
				continue;
			}

			if (strncmp(src, "mathrm", 6) == 0) {
				while (*src != '}') {
					*dst++ = *src++;
				}
				continue;
			}

			if (strncmp(src, "color", 5) == 0) {
				while (*src != '}') {
					*dst++ = *src++;
				}
				continue;
			}

			// アルファベット以外のものが来たらコマンド部分は終わり
			while ( (*src >= 0x41 && *src <=0x5a) || (*src>=0x61 && *src <= 0x7a) ) {
				*dst++ = *src++;
			}
			continue;
		}

		if (*src == 'g') {
			src++;
			if ((*src >= 0x41 && *src <= 0x5a) || (*src >= 0x61 && *src <= 0x7a)) {
				strcpy(dst, "\\varg ");
				dst += 6;
			} else {
				strcpy(dst, "\\varg");
				dst += 5;
			}
			continue;
		}

		if (*src == 'v') {
			src++;
			if ((*src >= 0x41 && *src <= 0x5a) || (*src >= 0x61 && *src <= 0x7a)) {
				strcpy(dst, "\\varv ");
				dst += 6;
			}
			else {
				strcpy(dst, "\\varv");
				dst += 5;
			}
			continue;
		}

		if (*src == 'y') {
			src++;
			if ((*src >= 0x41 && *src <= 0x5a) || (*src >= 0x61 && *src <= 0x7a)) {
				strcpy(dst, "\\vary ");
				dst += 6;
			}
			else {
				strcpy(dst, "\\vary");
				dst += 5;
			}
			continue;
		}

		*dst++ = *src++;
	}
	*dst = 0;
}

/* ************************************************** */
/*      SVGファイル作成                               */
/* ************************************************** */
static void make_svg(void) {

	int i;
	int f_failed;
	static char dvi_name[256];
	static char svg_name[256];
	static char cmd[256];

	while (1) {

		f_failed = false;

		for (i = 0; i < inline_num; i++) {
			sprintf(dvi_name, "%s\\%s\\%s\\eq%s\\i%03d.dvi", WORK_FOLDER, field, title, unique, i+1);
			if (existFile(dvi_name) == 0) {
				f_failed = true;
				break;
			}
		}
		if (f_failed) {
			printf("failed at inline %d/%d\n", i+1, inline_num);
			Sleep(2000);
			continue;
		}

		for (i = 0; i < display_num; i++) {
			sprintf(dvi_name, "%s\\%s\\%s\\eq%s\\d%03d.dvi", WORK_FOLDER, field, title, unique, i+1);
			if (existFile(dvi_name) == 0) {
				f_failed = true;
				break;
			}
		}
		if (f_failed) {
			printf("failed at display %d/%d\n", i + 1, display_num);
			Sleep(1000);
			continue;
		}

		break;
	}

	//printf("dvisvgm start\n");
	// 全てのファイルが完成した
	Sleep(1000); // 念のためもう1秒待つ


	for (i = 0; i < inline_num; i++) {
		sprintf(dvi_name, "%s\\%s\\%s\\eq%s\\i%03d.dvi", WORK_FOLDER, field, title, unique, i+1);
		sprintf(svg_name, "%s\\%s\\%s\\eq%s\\i%03d.svg",  DST_FOLDER, field, title, unique, i+1);
		sprintf(cmd, "start dvisvgm -n --bbox=0.1 --exact --fontmap=kanjix.map -o %s %s", svg_name, dvi_name);
		system(cmd);
	}

	for (i = 0; i < display_num; i++) {
		sprintf(dvi_name, "%s\\%s\\%s\\eq%s\\d%03d.dvi", WORK_FOLDER, field, title, unique, i+1);
		sprintf(svg_name, "%s\\%s\\%s\\eq%s\\d%03d.svg", DST_FOLDER, field, title, unique, i+1);
		sprintf(cmd, "start dvisvgm -n --bbox=0.1 --exact --fontmap=kanjix.map -o %s %s", svg_name, dvi_name);
		system(cmd);
	}
	//printf("dvisvgm end\n");

}

/* ************************************************** */
/*      SVGファイルの大きさ読み取り                   */
/* ************************************************** */
static void read_svg(void) {

	int i;
	int f_failed;
	static char svg_name[256];

	while (1) {

		f_failed = false;

		for (i = 0; i < inline_num; i++) {
			sprintf(svg_name, "%s\\%s\\%s\\eq%s\\i%03d.svg", DST_FOLDER, field, title, unique, i+1);
			if (existFile(svg_name) == 0) {
				f_failed = true;
				break;
			}
		}
		if (f_failed) {
			Sleep(500);
			continue;
		}

		for (i = 0; i < display_num; i++) {
			sprintf(svg_name, "%s\\%s\\%s\\eq%s\\d%03d.svg", DST_FOLDER, field, title, unique, i+1);
			if (existFile(svg_name) == 0) {
				f_failed = true;
				break;
			}
		}
		if (f_failed) {
			Sleep(500);
			continue;
		}
		break;
	}

	//printf("svg reading start\n");
	// 全てのファイルが完成した
	Sleep(1000); // 念のためもう1秒待つ

	for (i = 0; i < inline_num; i++) {
		sprintf(svg_name, "%s\\%s\\%s\\eq%s\\i%03d.svg", DST_FOLDER, field, title, unique, i+1);
		analyze_svg(svg_name, 1, i+1);
	}

	for (i = 0; i < display_num; i++) {
		sprintf(svg_name, "%s\\%s\\%s\\eq%s\\d%03d.svg", DST_FOLDER, field, title, unique, i+1);
		analyze_svg(svg_name, 0, i+1);
	}
}

/* ************************************************** */
/*      ファイルの存在確認                               */
/* ************************************************** */
int existFile(const char* path){

	FILE* fp = fopen(path, "r");
	if (fp == NULL) {
		return 0;
	}

	fclose(fp);
	return 1;
}
/* ************************************************** */
/*      SVGからサイズを取り出す                       */
/* ************************************************** */
static void analyze_svg( char* fname, int f_inline, int num) {

	char* svg_buf;
	char* src;
	char buf[16];
	float x;
	float y;
	int x_found;
	int y_found;
	int idx;

	svg_buf = read_file(fname);
	if (svg_buf == NULL) {
		printf("SVGファイルが開けません(%s)\n", fname);
		return;
	}

	src = svg_buf;

	// 念のため初期化
	x = 0;
	y = 0;

	// これから中身を読み出す。
	x_found = false;
	y_found = false;

	while (*src != 0) {

		if (x_found && y_found) break;

		if (*src == 'w') {
			if (strncmp(src, "width", 5) == 0) {
				// 多分この後で "='"が来るはずなので
				src += 7;
				idx = 0;
				while (*(src + idx) != 'p') { // ptの直前までほしい
					idx++;
				}
				strncpy(buf, src, idx);
				buf[idx] = 0;
				//printf("width [%s]\n", buf);
				x = (float)atof(buf);
				src += idx;
				x_found = true;
				continue;
			}
		}

		if (*src == 'h') {
			if (strncmp(src, "height", 6) == 0) {
				// 多分この後で "='"が来るはずなので
				src += 8;
				idx = 0;
				while (*(src + idx) != 'p') { // ptの直前までほしい
					idx++;
				}
				strncpy(buf, src, idx);
				buf[idx] = 0;
				//printf("height [%s]\n", buf);
				y = (float)atof(buf);
				src += idx;
				y_found = true;
				continue;
			}
		}

		src++;
	}


	// 結果を記録
	if (f_inline) {
		inline_x[num-1] = x;
		inline_y[num-1] = y;
	} else {
		display_x[num-1] = x;
		display_y[num-1] = y;
	}

	free(svg_buf);

}


/* ************************************************** */
/*      CSSファイル作成                               */
/* ************************************************** */
static void make_css(void) {

	int i;
	static char css_buf[16384];
	char fname[128];
	char buf[128];
	int idx;

	// スマホ用記述
	idx = 0;

	sprintf(buf, ".eq_v1 {\n  position: relative;\n  vertical-align: -1px;\n}\n\n");
	strcpy(css_buf + idx, buf);
	idx += strlen(buf);

	sprintf(buf, ".eq_v2 {\n  position: relative;\n  vertical-align: -5px;\n}\n\n");
	strcpy(css_buf + idx, buf);
	idx += strlen(buf);

	sprintf(buf, ".eq_v3 {\n  position: relative;\n  vertical-align: -1.3px;\n}\n\n");
	strcpy(css_buf + idx, buf);
	idx += strlen(buf);

	sprintf(buf, ".eq_v4 {\n  position: relative;\n  vertical-align: -2.5px;\n}\n\n");
	strcpy(css_buf + idx, buf);
	idx += strlen(buf);

	for (i = 0; i < inline_num; i++) {
		sprintf(buf, ".eq_i%d {\n  width: %.2fpx;\n  height: %.2fpx;\n}\n\n", i+1, inline_x[i] * 1.6, inline_y[i] * 1.6);
		strcpy(css_buf + idx, buf);
		idx += strlen(buf);
	}

	for (i = 0; i < display_num; i++) {
		sprintf(buf, ".eq_d%d {\n  width: %.2fpx;\n  height: %.2fpx;\n}\n\n", i+1, display_x[i] * 1.6, display_y[i] * 1.6);
		strcpy(css_buf + idx, buf);
		idx += strlen(buf);
	}

	sprintf(fname, "%s\\%s\\%s\\eq%s\\sp.css", DST_FOLDER, field, title, unique);
	write_html(fname, css_buf);


	// PC用記述
	idx = 0;

	sprintf(buf, ".eq_v1 {\n  position: relative;\n  vertical-align: -1px;\n}\n\n");
	strcpy(css_buf + idx, buf);
	idx += strlen(buf);

	sprintf(buf, ".eq_v2 {\n  position: relative;\n  vertical-align: -5.5px;\n}\n\n");
	strcpy(css_buf + idx, buf);
	idx += strlen(buf);

	sprintf(buf, ".eq_v3 {\n  position: relative;\n  vertical-align: -2px;\n}\n\n");
	strcpy(css_buf + idx, buf);
	idx += strlen(buf);

	sprintf(buf, ".eq_v4 {\n  position: relative;\n  vertical-align: -3px;\n}\n\n");
	strcpy(css_buf + idx, buf);
	idx += strlen(buf);

	for (i = 0; i < inline_num; i++) {
		sprintf(buf, ".eq_i%d {\n  width: %.2fpx;\n  height: %.2fpx;\n}\n\n", i + 1, inline_x[i] * 1.85, inline_y[i] * 1.85);
		strcpy(css_buf + idx, buf);
		idx += strlen(buf);
	}

	for (i = 0; i < display_num; i++) {
		sprintf(buf, ".eq_d%d {\n  width: %.2fpx;\n  height: %.2fpx;\n}\n\n", i + 1, display_x[i] * 1.85, display_y[i] * 1.85);
		strcpy(css_buf + idx, buf);
		idx += strlen(buf);
	}

	sprintf(fname, "%s\\%s\\%s\\eq%s\\pc.css", DST_FOLDER, field, title, unique);
	write_html(fname, css_buf);
}