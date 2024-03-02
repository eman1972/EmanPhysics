/* ************************************** */
/*                                        */
/*   RicciCount Ver 0.93                  */
/*      DATE: 2005.5.17                   */
/*      Programed by Katsuhiko Hiroe      */
/*              for EMAN's physics        */
/* ************************************** */


/* ���b�`�e���\���̓�̓Y�����ƃt�@�C�����A���v�R�̈�����n���Ă��ƁA*/
/* �w�肵���t�@�C�����Ńt�@�C�����쐬���A�����ɁA�w�肵���e���\��������    */
/* �W�J����Tex�`���ŏ��������v���O�����ł���B                             */
/* ���łɁA�W�J��Ƃœ������v������ʕ\������B                        */



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

/* �W���f�[�^�i���̃v���O�����̍ŏd�v�����j */
static char ddg_factor[4][4][4][4][4][4];             /* �Q�K������ */
static char mdg_factor[4][4][4][4][4][4][4][4][4][4]; /* �P�K������ */



/* ���v�f�[�^�i�v�Z�̂��łɑS�̂̕��͋C��T�邽�߂̂��́j */
static char ddg_check[4][4][4][4][4][4];
static char mdg_check[4][4][4][4][4][4][4][4][4][4];

int total;   /* �������� */
int effect;  /* �L������ */
int dead0;   /* �W�J���ɏo�ė��������Ȃ��������̐� */
int dead2;   /* �Q�����ł����������ď��ł����g�̐� */
int dead3;   /* �R�����ł����������ď��ł����g�̐� */
int dead4;   /* �S�����ł����������ď��ł����g�̐� */
int dead5;   /* �T�����ł����������ď��ł����g�̐� */
int dead6;   /* �U�����ł����������ď��ł����g�̐� */
int dead7;   /* �V�����ł����������ď��ł����g�̐� */
int dead8;   /* �W�����ł����������ď��ł����g�̐� */
int dead9;   /* �X�����ł����������ď��ł����g�̐� */
int dead10;  /* �P�O�����ł����������ď��ł����g�̐� */
int dead11;  /* �P�P�����ł����������ď��ł����g�̐� */
int dead12;  /* �P�Q���ȏオ�ł����������ď��ł����g�̐� */

int surv1;   /* �P�ƂŐ����c�������̐� */
int surv2;   /* �Q�������킳���Đ����c�����g�̐� */
int surv3;   /* �R�������킳���Đ����c�����g�̐� */
int surv4;   /* �S�������킳���Đ����c�����g�̐� */
int surv5;   /* �T�������킳���Đ����c�����g�̐� */
int surv6;   /* �U�������킳���Đ����c�����g�̐� */
int surv7;   /* �V�������킳���Đ����c�����g�̐� */
int surv8;   /* �W�������킳���Đ����c�����g�̐� */
int surv9;   /* �X�������킳���Đ����c�����g�̐� */
int surv10;  /* �P�O�������킳���Đ����c�����g�̐� */
int surv11;  /* �P�P�������킳���Đ����c�����g�̐� */
int surv12;  /* �P�Q���ȏオ���킳���Đ����c�����g�̐� */


/* ****************************************** */
/*   ���C��                                   */
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

    /* �W���f�[�^�̃N���A */
    memset( ddg_factor, 0, sizeof(ddg_factor) );
    memset( mdg_factor, 0, sizeof(mdg_factor)  );
    memset( ddg_check, 0, sizeof(ddg_check)  );
    memset( mdg_check, 0, sizeof(mdg_check)  );

    /* �W�J�� */
    printf( "Expanding...\n" );
    ricci(a,b);

    /* ���v�쐬�� �y�� TeX �f�[�^�쐬�� */
    printf( "Counting...\n" );
    make_tex_file();

    if( fp != NULL ){
        fclose( fp );
    }
    return 0;
}



/* ****************************************** */
/*   �v�ʂ̓Y���̕��т𓝈ꂷ��               */
/*  �Y���𐔎��̏��������ɕ��ׂ�B            */
/*  ��F g_31 �� g_13 �ɕϊ�����B            */
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
/*   �v�ʂ̏��Ԃ̓���ւ��𓯈ꎋ����B       */
/*   g_{ab} �� g_{cd} ����������A            */
/*  ab �� cd �����ꂼ��񌅂̐����ƌ��āA     */
/*  ���������ɕ��ׂ�B                        */
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
/*   �Δ����̏��Ԃ̓���ւ��𓯈ꎋ����B          */
/*   ��g_{ab}/��x^c �� ��g_{de}/��x^f ����������A */
/*  abc �� def �����ꂼ��O���̐����ƌ��āA        */
/*  ���������ɕ��ׂ�B                             */
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
/*   ���b�`�E�e���\�� R_{ab} ���v�Z           */
/* ****************************************** */
void ricci( int a, int b ){

    int i;

    for( i=0;i<=3; i++ ){
        riemann( i, a, i, b );
    }
}


/* ****************************************** */
/*   ���[�}���E�e���\�� R^a_{b,cd} ���v�Z     */
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
/*   �N���X�g�b�t�F���L���̔������v�Z         */
/*        ��_d ��^a_{bc}                      */
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
/*   �N���X�g�b�t�F���L���̔�����                         */
/*      ��K�����̂Ƃ�����v�Z                            */
/*   (1/2) (��g^{ab}/��x_c) (��g_{de}/��x_f)              */
/*     g^{ab}�̕����� g_{ij} �ɕϊ�����K�v������         */
/*     add_dg()��(1/4)�P�ʂŉ��Z����̂łQ�{���Ă���B    */
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
/*   �N���X�g�b�t�F���L���ǂ����̐ς��v�Z                 */
/*        ��^a_{bc} ��^d_{ef}                             */
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
/*   �Q�K�����̍������Z�B  (1/2)����P�ʂƂ���B          */
/*   (1/2) g^{ab} {��^2 g_{cd}/(��x^e ��x^f)}             */
/* ****************************************************** */
void add_ddg(int sign, int a, int b, int c, int d, int e, int f ){

    swap_idx( &a, &b );
    swap_idx( &c, &d );
    swap_idx( &e, &f );
    ddg_factor[a][b][c][d][e][f] += sign;
    ddg_check[a][b][c][d][e][f] ++;
}

/* ************************************************************ */
/*   �P�K�����̍������Z�B  (1/4)����P�ʂƂ���B                */
/*   (1/4) g^{ab} g^{cd} (��g_{ef}/��x^g)(��g_{hi}/��x^j)       */
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
/*   �d���f�[�^���l�����Ȃ���TEX�\�[�X�쐬    */
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
/*   �Q�K�������̏W�v    */
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
/*   �P�K�����̐ς��W�v    */
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
/*   �Q�K�����̍��� TeX �`���ŏo��                         */
/*  (1/2) g^{ab} (��^2g_{cd}/��x^e ��x^f)                  */
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
/*   �P�K�����̐ς̍��� TeX �`���ŏo��                     */
/*  (1/4) g^{ab} g^{cd} (��g_{ef}/��x^g) (��g_{hi}/��x^j)  */
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
/*   �t�@�C���ɍ����������ށB                         */
/*  �S���������ޓx�Ɏ����I�ɉ��s������B            */
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
/*   �W�v�f�[�^�̃N���A                       */
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
/*   �Q�K�����̍��ɂ��Ă̓��v�������Z                  */
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
/*   �P�K�����̍��ɂ��Ă̓��v�������Z                  */
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
/*   ���v�f�[�^�̌��ʕ\���B                           */
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


