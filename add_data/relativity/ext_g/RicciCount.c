/* ************************************** */
/*                                        */
/*   RicciCount Ver 0.93                  */
/*      DATE: 2005.5.17                   */
/*      Programed by Katsuhiko Hiroe      */
/*              for EMAN's physics        */
/* ************************************** */


/* リッチテンソルの二つの添え字とファイル名、合計３つの引数を渡してやると、*/
/* 指定したファイル名でファイルを作成し、そこに、指定したテンソル成分の    */
/* 展開式をTex形式で書き下すプログラムである。                             */
/* ついでに、展開作業で得た統計情報も画面表示する。                        */



#include <stdio.h>
#include <string.h>


void swap_idx( int *a, int *b );
void swap_metric( int *a, int *b, int *c, int *d );
void swap_partial( int *a, int *b, int *c, int *d, int *e, int *f );

void ricci( int a, int b );
void riemann( int a, int b, int c, int d );

void dif_cris( int sign, int d, int a, int b, int c );
void dif_cris_sub( int sign, int a, int b, int c, int d, int e, int f );
void multi_cris( int sign, int a, int b, int c, int d, int e, int f );

void add_ddg(int sign, int a, int b, int c, int d, int e, int f );
void add_mdg(int sign, int a, int b, int c, int d, int e, int f, int g, int h, int i, int j );

void make_tex_file( void );
void make_ddg( void );
void make_mdg( void );

void write_ddg( int a, int b, int c, int d, int e, int f );
void write_mdg( int a, int b, int c, int d, int e, int f, int g, int h, int i, int j );
void tex_write( char *buf );

void reset_counters( void );

void check_ddg( int a, int b, int c, int d, int e, int f );
void check_mdg( int a, int b, int c, int d, int e, int f, int g, int h, int i, int j );

void print_check_result( void );

FILE * fp = NULL;
int  line;

/* 係数データ（このプログラムの最重要部分） */
static char ddg_factor[4][4][4][4][4][4];             /* ２階微分項 */
static char mdg_factor[4][4][4][4][4][4][4][4][4][4]; /* １階微分項 */



/* 統計データ（計算のついでに全体の雰囲気を探るためのもの） */
static char ddg_check[4][4][4][4][4][4];
static char mdg_check[4][4][4][4][4][4][4][4][4][4];

int total;   /* 調査総数 */
int effect;  /* 有効項数 */
int dead0;   /* 展開時に出て来さえしなかった項の数 */
int dead2;   /* ２項が打ち消しあって消滅した組の数 */
int dead3;   /* ３項が打ち消しあって消滅した組の数 */
int dead4;   /* ４項が打ち消しあって消滅した組の数 */
int dead5;   /* ５項が打ち消しあって消滅した組の数 */
int dead6;   /* ６項が打ち消しあって消滅した組の数 */
int dead7;   /* ７項が打ち消しあって消滅した組の数 */
int dead8;   /* ８項が打ち消しあって消滅した組の数 */
int dead9;   /* ９項が打ち消しあって消滅した組の数 */
int dead10;  /* １０項が打ち消しあって消滅した組の数 */
int dead11;  /* １１項が打ち消しあって消滅した組の数 */
int dead12;  /* １２項以上が打ち消しあって消滅した組の数 */

int surv1;   /* 単独で生き残った項の数 */
int surv2;   /* ２項が合わさって生き残った組の数 */
int surv3;   /* ３項が合わさって生き残った組の数 */
int surv4;   /* ４項が合わさって生き残った組の数 */
int surv5;   /* ５項が合わさって生き残った組の数 */
int surv6;   /* ６項が合わさって生き残った組の数 */
int surv7;   /* ７項が合わさって生き残った組の数 */
int surv8;   /* ８項が合わさって生き残った組の数 */
int surv9;   /* ９項が合わさって生き残った組の数 */
int surv10;  /* １０項が合わさって生き残った組の数 */
int surv11;  /* １１項が合わさって生き残った組の数 */
int surv12;  /* １２項以上が合わさって生き残った組の数 */


/* ****************************************** */
/*   メイン                                   */
/* ****************************************** */
int main( int argc, char *argv[] ){

    int a, b;

    printf( "\nRicciCount Ver 0.90 by Katsuhiko Hiroe 2005\n" );
    if( argc != 4 ){
        printf( "(Usage): RicciCount u v [tex_file]\n" );
        return 0;
    }

    fp = fopen( argv[3], "w" );
    if( fp == NULL ){
        printf( "Can't create File: %s \n", argv[3] );
        return 0;
    }

    a = -1;
    b = -1;
    sscanf( argv[1], "%d", &a );
    sscanf( argv[2], "%d", &b );
    if( (a < 0)||(a > 3)||(b < 0)||(b > 3) ){
        printf("Error: bad parameters\n");
        return 0;
    }

    line = 0;

    /* 係数データのクリア */
    memset( ddg_factor, 0, sizeof(ddg_factor) );
    memset( mdg_factor, 0, sizeof(mdg_factor)  );
    memset( ddg_check, 0, sizeof(ddg_check)  );
    memset( mdg_check, 0, sizeof(mdg_check)  );

    /* 展開中 */
    printf( "Expanding...\n" );
    ricci(a,b);

    /* 統計作成中 及び TeX データ作成中 */
    printf( "Counting...\n" );
    make_tex_file();

    if( fp != NULL ){
        fclose( fp );
    }
    return 0;
}



/* ****************************************** */
/*   計量の添字の並びを統一する               */
/*  添字を数字の小さい順に並べる。            */
/*  例： g_31 は g_13 に変換する。            */
/* ****************************************** */
void swap_idx( int *a, int *b ){

    int tmp;

    if( *a > *b) {
        tmp = *a;
        *a = *b;
        *b = tmp;
    }
}

/* ****************************************** */
/*   計量の順番の入れ替えを同一視する。       */
/*   g_{ab} と g_{cd} があったら、            */
/*  ab と cd をそれぞれ二桁の数字と見て、     */
/*  小さい順に並べる。                        */
/* ****************************************** */
void swap_metric( int *a, int *b, int *c, int *d ){

    int tmp;
    int metric1, metric2;

    metric1 = *a * 10 + *b;
    metric2 = *c * 10 + *d;

    if( metric1 > metric2 ){
        tmp = *a;
        *a = *c;
        *c = tmp;

        tmp = *b;
        *b = *d;
        *d = tmp;
    }
}

/* *********************************************** */
/*   偏微分の順番の入れ替えを同一視する。          */
/*   ∂g_{ab}/∂x^c と ∂g_{de}/∂x^f があったら、 */
/*  abc と def をそれぞれ三桁の数字と見て、        */
/*  小さい順に並べる。                             */
/* *********************************************** */
void swap_partial( int *a, int *b, int *c, int *d, int *e, int *f ){

    int tmp;
    int part1, part2;

    part1 = *a * 100 + *b * 10 + *c;
    part2 = *d * 100 + *e * 10 + *f;

    if( part1 > part2 ){
        tmp = *a;
        *a = *d;
        *d = tmp;

        tmp = *b;
        *b = *e;
        *e = tmp;

        tmp = *c;
        *c = *f;
        *f = tmp;
    }
}

/* ****************************************** */
/*   リッチ・テンソル R_{ab} を計算           */
/* ****************************************** */
void ricci( int a, int b ){

    int i;

    for( i=0;i<=3; i++ ){
        riemann( i, a, i, b );
    }
}


/* ****************************************** */
/*   リーマン・テンソル R^a_{b,cd} を計算     */
/* ****************************************** */
void riemann( int a, int b, int c, int d ){

    int i;

    dif_cris( 1, c,a,b,d);

    dif_cris(-1, d,a,b,c);

    for( i=0; i<=3; i++ ){
        multi_cris( 1, i,b,d, a,i,c);
    }

    for( i=0; i<=3; i++ ){
        multi_cris(-1, i,b,c, a,i,d);
    }
}


/* ****************************************** */
/*   クリストッフェル記号の微分を計算         */
/*        ∂_d Γ^a_{bc}                      */
/* ****************************************** */
void dif_cris( int sign, int d, int a, int b, int c ){

    int i;
    
    for( i=0; i<=3; i++ ){
        dif_cris_sub( sign, a,i,d, i,c,b );
        dif_cris_sub( sign, a,i,d, i,b,c );
        dif_cris_sub(-sign, a,i,d, b,c,i );

        add_ddg( sign, a,i, i,c, b,d );
        add_ddg( sign, a,i, i,b, c,d );
        add_ddg(-sign, a,i, b,c, i,d );
    }
}

/* ****************************************************** */
/*   クリストッフェル記号の微分の                         */
/*      一階微分のところを計算                            */
/*   (1/2) (∂g^{ab}/∂x_c) (∂g_{de}/∂x_f)              */
/*     g^{ab}の部分を g_{ij} に変換する必要がある         */
/*     add_dg()は(1/4)単位で加算するので２倍している。    */
/* ****************************************************** */
void dif_cris_sub( int sign, int a, int b, int c, int d, int e, int f ){

    int i, j;

    for( i=0; i<=3; i++ ){
        for( j=0; j<=3; j++ ){
            add_mdg( -2*sign, a,i, b,j, i,j,c, d,e,f );
        }
    }
}


/* ****************************************************** */
/*   クリストッフェル記号どうしの積を計算                 */
/*        Γ^a_{bc} Γ^d_{ef}                             */
/* ****************************************************** */
void multi_cris( int sign, int a, int b, int c, int d, int e, int f ){

    int i,j;

    for( i=0; i<=3; i++ ){
        for( j=0; j<=3; j++ ){
            add_mdg( sign, a,i, d,j, i,c,b, j,f,e );
            add_mdg( sign, a,i, d,j, i,c,b, j,e,f );
            add_mdg(-sign, a,i, d,j, i,c,b, e,f,j );

            add_mdg( sign, a,i, d,j, i,b,c, j,f,e );
            add_mdg( sign, a,i, d,j, i,b,c, j,e,f );
            add_mdg(-sign, a,i, d,j, i,b,c, e,f,j );

            add_mdg(-sign, a,i, d,j, b,c,i, j,f,e );
            add_mdg(-sign, a,i, d,j, b,c,i, j,e,f );
            add_mdg( sign, a,i, d,j, b,c,i, e,f,j );
        }
    }
}

/* ****************************************************** */
/*   ２階微分の項を加算。  (1/2)を一単位とする。          */
/*   (1/2) g^{ab} {∂^2 g_{cd}/(∂x^e ∂x^f)}             */
/* ****************************************************** */
void add_ddg(int sign, int a, int b, int c, int d, int e, int f ){

    swap_idx( &a, &b );
    swap_idx( &c, &d );
    swap_idx( &e, &f );
    ddg_factor[a][b][c][d][e][f] += sign;
    ddg_check[a][b][c][d][e][f] ++;
}

/* ************************************************************ */
/*   １階微分の項を加算。  (1/4)を一単位とする。                */
/*   (1/4) g^{ab} g^{cd} (∂g_{ef}/∂x^g)(∂g_{hi}/∂x^j)       */
/* ************************************************************ */
void add_mdg(int sign, int a, int b, int c, int d, int e, int f, int g, int h, int i, int j ){

    swap_idx( &a, &b );
    swap_idx( &c, &d );
    swap_idx( &e, &f );
    swap_idx( &h, &i );
    swap_metric( &a, &b, &c, &d );
    swap_partial( &e, &f, &g, &h, &i, &j );
    mdg_factor[a][b][c][d][e][f][g][h][i][j] += sign;
    mdg_check[a][b][c][d][e][f][g][h][i][j] ++;
}



/* ************************************************************* */
/* ************************************************************* */
/* ************************************************************* */



/* ****************************************** */
/*   重複データを考慮しながらTEXソース作成    */
/* ****************************************** */
void make_tex_file( void ){

    fprintf( fp, "\\documentclass{article}\n" );
    fprintf( fp, "\\begin{document}\n" );
    fprintf( fp, "\\begin{eqnarray}\n" );

    make_ddg();
    make_mdg();

    fprintf( fp, "\\nonumber \\\\ \n" );
    fprintf( fp, "\\end{eqnarray}\n" );
    fprintf( fp, "\\end{document}\n" );
}


/* ****************************************** */
/*   ２階微分項の集計    */
/* ****************************************** */
void make_ddg( void ){

    int a,b,c,d,e,f;

    reset_counters();

    for( a=0; a<=3; a++ ){
      for( b=0; b<=3; b++ ){
        for( c=0; c<=3; c++ ){
          for( d=0; d<=3; d++ ){
            for( e=0; e<=3; e++ ){
              for( f=0; f<=3; f++ ){
                  write_ddg( a,b,c,d,e,f );
                  check_ddg( a,b,c,d,e,f );
              }
            }
          }
        }
      }
    }

    print_check_result();

}



/* ****************************************** */
/*   １階微分の積を集計    */
/* ****************************************** */
void make_mdg( void ){

    int a,b,c,d,e,f,g,h,i,j;

    reset_counters();

    for( a=0; a<=3; a++ ){
      for( b=0; b<=3; b++ ){
        for( c=0; c<=3; c++ ){
          for( d=0; d<=3; d++ ){
            for( e=0; e<=3; e++ ){
              for( f=0; f<=3; f++ ){
                for( g=0; g<=3; g++ ){
                  for( h=0; h<=3; h++ ){
                    for( i=0; i<=3; i++ ){
                      for( j=0; j<=3; j++ ){
                          write_mdg( a,b,c,d,e,f,g,h,i,j );
                          check_mdg( a,b,c,d,e,f,g,h,i,j );
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }

    print_check_result();
}



/* ******************************************************* */
/*   ２階微分の項を TeX 形式で出力                         */
/*  (1/2) g^{ab} (∂^2g_{cd}/∂x^e ∂x^f)                  */
/* ******************************************************* */
void write_ddg( int a, int b, int c, int d, int e, int f ){

    int factor;
    char buf[256];
    char tmp[256];
    
    buf[0] = 0;
    
    factor = ddg_factor[a][b][c][d][e][f];
    
    if( factor == 0 ){
        return;
    }
    
    if( factor > 0 ){
        strcpy( buf, "+ " );
    } else {
        strcpy( buf, "- " );
        factor = -factor;
    }
    
    if( factor%2 == 0 ){
        if( factor/2 != 1 ){
            sprintf( tmp, "%d ", factor/2 );
            strcat( buf, tmp );
        }
    }
    else {
        sprintf( tmp, "\\frac{%d}{2} ", factor );
        strcat( buf, tmp );
    }
    
    sprintf( tmp, "g^{%d%d} ", a, b );
    strcat( buf, tmp );

    sprintf( tmp, "\\frac{\\partial^2 g_{%d%d}}{\\partial x^{%d} \\partial x^{%d}} ", c, d, e, f );
    strcat( buf, tmp );

    tex_write( buf );

}


/* ******************************************************* */
/*   １階微分の積の項を TeX 形式で出力                     */
/*  (1/4) g^{ab} g^{cd} (∂g_{ef}/∂x^g) (∂g_{hi}/∂x^j)  */
/* ******************************************************* */
void write_mdg( int a, int b, int c, int d, int e, int f, int g, int h, int i, int j ){

    int factor;
    char buf[256];
    char tmp[256];
    
    buf[0] = 0;
    
    factor = mdg_factor[a][b][c][d][e][f][g][h][i][j];
    
    if( factor == 0 ){
        return;
    }
    
    if( factor > 0 ){
        strcpy( buf, "+ " );
    } else {
        strcpy( buf, "- " );
        factor = -factor;
    }
    
    if( factor%4 == 0 ){
        if( factor != 4 ){
            sprintf( tmp, "%d", factor/4 );
            strcat( buf, tmp );
        }
    }
    else if( factor%2 == 0 ){
        sprintf( tmp, "\\frac{%d}{2} ", factor/2 );
        strcat( buf, tmp );
    }
    else {
        sprintf( tmp, "\\frac{%d}{4} ", factor );
        strcat( buf, tmp );
    }
    
    if( (a==c)&&(b==d) ){
        sprintf( tmp, "(g^{%d%d})^2", a, b );
        strcat( buf, tmp );
    } else {
        sprintf( tmp, "g^{%d%d} g^{%d%d}", a, b, c, d );
        strcat( buf, tmp );
    }

    if( (e==h)&&(f==i)&&(g==j) ){
        sprintf( tmp, "\\left( \\frac{\\partial g_{%d%d}}{\\partial x^{%d}} \\right)^2 ", e, f, g );
        strcat( buf, tmp );
    } else {
        sprintf( tmp, "\\frac{\\partial g_{%d%d}}{\\partial x^{%d}} ", e, f, g );
        strcat( buf, tmp );
        sprintf( tmp, "\\frac{\\partial g_{%d%d}}{\\partial x^{%d}} ", h, i, j );
        strcat( buf, tmp );
    }

    tex_write( buf );
}


/* ************************************************** */
/*   ファイルに項を書き込む。                         */
/*  ４項書き込む度に自動的に改行を入れる。            */
/* ************************************************** */
void tex_write( char *buf ){

    if( line == 4 ){
        line = 0;
        fprintf( fp, "\\nonumber \\\\ \n" );
    }
    if( line == 0 ){
        fprintf( fp, "& & " );
    }

    fprintf( fp, buf );
    line ++;
}



/* ****************************************** */
/*   集計データのクリア                       */
/* ****************************************** */
void reset_counters( void ){

    total = 0;
    effect = 0;
    dead0 = 0;
    dead2 = 0;
    dead3 = 0;
    dead4 = 0;
    dead5 = 0;
    dead6 = 0;
    dead7 = 0;
    dead8 = 0;
    dead9 = 0;
    dead10 = 0;
    dead11 = 0;
    dead12 = 0;

    surv1 = 0;
    surv2 = 0;
    surv3 = 0;
    surv4 = 0;
    surv5 = 0;
    surv6 = 0;
    surv7 = 0;
    surv8 = 0;
    surv9 = 0;
    surv10 = 0;
    surv11 = 0;
    surv12 = 0;
}

/* ******************************************************* */
/*   ２階微分の項についての統計情報を加算                  */
/* ******************************************************* */
void check_ddg( int a, int b, int c, int d, int e, int f ){

    total ++;

    if( ddg_factor[a][b][c][d][e][f] == 0 ){
        switch( ddg_check[a][b][c][d][e][f] ){
            case 0: dead0 ++; break;
            case 2: dead2 ++; break;
            case 3: dead3 ++; break;
            case 4: dead4 ++; break;
            case 5: dead5 ++; break;
            case 6: dead6 ++; break;
            case 7: dead7 ++; break;
            case 8: dead8 ++; break;
            case 9: dead9 ++; break;
            case 10: dead10 ++; break;
            case 11: dead11 ++; break;
            default: dead12 ++; break;
        }
    } else {
        effect ++;
        switch( ddg_check[a][b][c][d][e][f] ){
            case 1: surv1 ++; break;
            case 2: surv2 ++; break;
            case 3: surv3 ++; break;
            case 4: surv4 ++; break;
            case 5: surv5 ++; break;
            case 6: surv6 ++; break;
            case 7: surv7 ++; break;
            case 8: surv8 ++; break;
            case 9: surv9 ++; break;
            case 10: surv10 ++; break;
            case 11: surv11 ++; break;
            default: surv12 ++; break;
        }
    }
}

/* ******************************************************* */
/*   １階微分の項についての統計情報を加算                  */
/* ******************************************************* */
void check_mdg( int a, int b, int c, int d, int e, int f, int g, int h, int i, int j ){

    total ++;
    if( mdg_factor[a][b][c][d][e][f][g][h][i][j] == 0 ){
        switch( mdg_check[a][b][c][d][e][f][g][h][i][j] ){
            case 0: dead0 ++; break;
            case 2: dead2 ++; break;
            case 3: dead3 ++; break;
            case 4: dead4 ++; break;
            case 5: dead5 ++; break;
            case 6: dead6 ++; break;
            case 7: dead7 ++; break;
            case 8: dead8 ++; break;
            case 9: dead9 ++; break;
            case 10: dead10 ++; break;
            case 11: dead11 ++; break;
            default: dead12 ++; break;
        }
    } else {
        effect ++;
        switch( mdg_check[a][b][c][d][e][f][g][h][i][j] ){
            case 1: surv1 ++; break;
            case 2: surv2 ++; break;
            case 3: surv3 ++; break;
            case 4: surv4 ++; break;
            case 5: surv5 ++; break;
            case 6: surv6 ++; break;
            case 7: surv7 ++; break;
            case 8: surv8 ++; break;
            case 9: surv9 ++; break;
            case 10: surv10 ++; break;
            case 11: surv11 ++; break;
            default: surv12 ++; break;
        }
    }
}

/* ************************************************** */
/*   統計データの結果表示。                           */
/* ************************************************** */
void print_check_result( void ){

    printf( "total: %d/%d\n", effect, total );
    printf( "dead0: %d\n", dead0 );
    printf( "dead2: %d\n", dead2 );
    printf( "dead3: %d\n", dead3 );
    printf( "dead4: %d\n", dead4 );
    printf( "dead5: %d\n", dead5 );
    printf( "dead6: %d\n", dead6 );
    printf( "dead7: %d\n", dead7 );
    printf( "dead8: %d\n", dead8 );
    printf( "dead9: %d\n", dead9 );
    printf( "dead10: %d\n", dead10 );
    printf( "dead11: %d\n", dead11 );
    printf( "dead12: %d\n", dead12 );

    printf( "surv1: %d\n", surv1 );
    printf( "surv2: %d\n", surv2 );
    printf( "surv3: %d\n", surv3 );
    printf( "surv4: %d\n", surv4 );
    printf( "surv5: %d\n", surv5 );
    printf( "surv6: %d\n", surv6 );
    printf( "surv7: %d\n", surv7 );
    printf( "surv8: %d\n", surv8 );
    printf( "surv9: %d\n", surv9 );
    printf( "surv10: %d\n", surv10 );
    printf( "surv11: %d\n", surv11 );
    printf( "surv12: %d\n", surv12 );
}


