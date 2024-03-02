///////////////////////////////////////////////////////////////
// 波束の観察
// Copyright (c) 2014 K.Hiroe

// 注意：素人が見様見真似で作ったものなので参考にすべきでない。


//////////////////////////
// 大域変数
//////////////////////////
var width = 640;
var height = 720;

var top_margin = 15;
var side_margin = 20;
var graph_width = 600;
var graph_height = 390;

var gap_graph_panel = 30;
var panel_height = 280;
var panel_side = 20;

var btn_size_x = 40;
var btn_size_y = 40;

var gap_x = 20
var gap_y = 30;
var gap_waves = 60;

var pnum = 600;  // 形状データの点の個数(graph_widthと同じになる)

var h_wave1 = 60;
var h_wave2 = 165;
var h_wave3 = 310;

var ctx;           // canvasコンテキスト
var timerID;
var PI;
var PI2;

var mouseX, mouseY;


var wave1 = [];
var wave2 = [];

var time = 0;

var amp1 = 33;
var amp2 = 33;

var len1 = 5;
var len2 = 4;

var vel1 = 9;
var vel2 = 10;

var phase1 = 0;
var phase2 = 0;

var ph1, ph2;

var btn = [];

var f_playing = true;
var f_vel_changed = false;

//////////////////////////
// 初期化
//////////////////////////
onload = function() {

  // 画面表示用コンテキストの取得
  var canvas = document.getElementById('drawfield');
  if (!canvas || !canvas.getContext) {
    alert("HTML5対応ブラウザでのみお試し頂けます");
    return false;
  }

  ctx = canvas.getContext('2d');

  // 大域変数の初期化
  init_vars();

  draw_frames();

  show_waves();

  regist_buttons();
  show_all_buttons();

  // マウスイベント関数の設定
  canvas.addEventListener('mousemove', updateMousePos, false);
  canvas.addEventListener('mouseout', resetMousePos, false);
  canvas.addEventListener('click', onClick, false);

  // タイマーイベント関数の設定
  timerID = setInterval('timerfunc()', 33);  // 秒間30フレーム

}

/////////////////////////////////
// 初期化
/////////////////////////////////
function init_vars() {

  mouseX = 100;
  mouseY = 100;

  PI = Math.PI;
  PI2 = 2 * Math.PI;

}

/////////////////////////////////
// 枠の準備
/////////////////////////////////
function draw_frames() {

  ctx.fillStyle="rgb(255,255,255)";
  ctx.fillRect(0,0,width,height);

  // メッセージ枠
  ctx.strokeStyle = "rgb(0,55,255)";
  ctx.lineCap = "round";
  ctx.strokeRect( panel_side, graph_height + gap_graph_panel, width - panel_side * 2, panel_height);

}

////////////////////////////////
// 波の表示
////////////////////////////////
function erase_waves(){

  ctx.fillStyle="rgb(255,255,255)";
  ctx.fillRect(0,0,width, top_margin + graph_height );

  // 文字
  ctx.fillStyle = "rgb(0, 0, 0)";
  ctx.font = "15px 'monospace'";
  ctx.fillText("wave1", 40, top_margin + h_wave1-44);
  ctx.fillText("wave2", 40, top_margin + h_wave2-44);
  ctx.fillText("wave1 + wave2", 40, top_margin + h_wave3-80);


  ctx.strokeStyle = "rgb(0,0,0)";
  ctx.lineWidth = 0.8;
  ctx.lineCap = "square";
  ctx.beginPath();

  // 左縦線
  ctx.moveTo( side_margin, top_margin );
  ctx.lineTo( side_margin, top_margin + graph_height );
  // 右縦線
  ctx.moveTo( width-side_margin, top_margin );
  ctx.lineTo( width-side_margin, top_margin + graph_height );

  // 中心横線
  ctx.moveTo( side_margin, top_margin + h_wave1 );
  ctx.lineTo( width-side_margin, top_margin + h_wave1 );
  // 中心目盛
  //ctx.moveTo( width/2, top_margin + h_wave1-6 );
  //ctx.lineTo( width/2, top_margin + h_wave1+6 );
  // 左小目盛
  ctx.moveTo( side_margin+graph_width/4, top_margin + h_wave1-3 );
  ctx.lineTo( side_margin+graph_width/4, top_margin + h_wave1+3 );
  // 右小目盛
  ctx.moveTo( side_margin+graph_width/4*3, top_margin + h_wave1-3 );
  ctx.lineTo( side_margin+graph_width/4*3, top_margin + h_wave1+3 );

  // 中心横線
  ctx.moveTo( side_margin, top_margin + h_wave2 );
  ctx.lineTo( width-side_margin, top_margin + h_wave2 );
  // 中心目盛
  //ctx.moveTo( width/2, top_margin + h_wave2-6 );
  //ctx.lineTo( width/2, top_margin + h_wave2+6 );
  // 左小目盛
  ctx.moveTo( side_margin+graph_width/4, top_margin + h_wave2-3 );
  ctx.lineTo( side_margin+graph_width/4, top_margin + h_wave2+3 );
  // 右小目盛
  ctx.moveTo( side_margin+graph_width/4*3, top_margin + h_wave2-3 );
  ctx.lineTo( side_margin+graph_width/4*3, top_margin + h_wave2+3 );

  // 中心横線
  ctx.moveTo( side_margin, top_margin + h_wave3 );
  ctx.lineTo( width-side_margin, top_margin + h_wave3 );
  // 中心目盛
  //ctx.moveTo( width/2, top_margin + h_wave3-6 );
  //ctx.lineTo( width/2, top_margin + h_wave3+6 );
  // 左小目盛
  ctx.moveTo( side_margin+graph_width/4, top_margin + h_wave3-3 );
  ctx.lineTo( side_margin+graph_width/4, top_margin + h_wave3+3 );
  // 右小目盛
  ctx.moveTo( side_margin+graph_width/4*3, top_margin + h_wave3-3 );
  ctx.lineTo( side_margin+graph_width/4*3, top_margin + h_wave3+3 );

  // Y軸
  ctx.moveTo( width/2, top_margin );
  ctx.lineTo( width/2, top_margin + graph_height );

  ctx.stroke();


}

function show_waves() {

  var i;
  var x;

  var k1;
  var k2;

  var w1;
  var w2;

  k1 = 3/Math.pow(1.08, len1)/40;
  k2 = 3/Math.pow(1.08, len2)/40;

  if( f_vel_changed ){
    f_vel_changed = false;
    phase1 = ph1 + phase1;
    phase2 = ph2 + phase2;
    time = 1;
  }

  w1 = vel1 * k1/10;
  w2 = vel2 * k2/10;

  ph1 = w1 * time;
  ph2 = w2 * time;

  erase_waves();

  // 波の線の設定
  ctx.beginPath();
  ctx.lineWidth = 2;
  ctx.lineCap = "round";
  ctx.strokeStyle = "rgb(0,0,255)";

  // 波の形の計算
  for( i=0; i<=pnum; i++ ){
    x = i - pnum/2;
    wave1[i] = amp1 * Math.sin( PI2 * (x*k1 - w1*time - phase1 ) );
    wave2[i] = amp2 * Math.sin( PI2 * (x*k2 - w2*time - phase2 ) );
  }


  ctx.moveTo( side_margin, -wave1[0] + top_margin + h_wave1 );

  for( i=1; i<pnum; i++ ){
    ctx.lineTo( side_margin + i, -wave1[i] + top_margin + h_wave1 );
  }

  ctx.moveTo( side_margin, -wave2[0] + top_margin + h_wave2 );

  for( i=1; i<pnum; i++ ){
    ctx.lineTo( side_margin + i, -wave2[i] + top_margin + h_wave2 );
  }

  ctx.moveTo( side_margin, -wave1[0] -wave2[0] + top_margin + h_wave3 );

  for( i=1; i<pnum; i++ ){
    ctx.lineTo( side_margin + i, -wave1[i] -wave2[i] + top_margin + h_wave3 );
  }

  ctx.stroke();
}


/////////////////////////////////
// 角丸四角形を描く
/////////////////////////////////
function fillRoundRect(l, t, w, h, r) {

  ctx.beginPath();
  ctx.arc(l + r, t + r, r, - PI, - 0.5 * PI, false);
  ctx.arc(l + w - r, t + r, r, - 0.5 * PI, 0, false);
  ctx.arc(l + w - r, t + h - r, r, 0, 0.5 * PI, false);
  ctx.arc(l + r, t + h - r, r, 0.5 * PI, PI, false);
  ctx.closePath();
  ctx.fill();
}


////////////////////////////////
// タイマーイベント関数
////////////////////////////////
function timerfunc() {

  if( f_playing ){
    time ++;
    show_waves();
  }
}

////////////////////////////////
// マウス位置の取得
////////////////////////////////
function updateMousePos(event) {

  var rect = event.target.getBoundingClientRect();
  mouseX = event.clientX - rect.left;
  mouseY = event.clientY - rect.top;

  check_buttons();

}

function resetMousePos() {

  mouseX = -100;
  mouseY = -100;
}

////////////////////////////////
// マウスが押されたときの処理
////////////////////////////////
function onClick(event) {

  n = -1;

  // どのボタンの上で押されたかを調べる
  var i;
  for( i=0; i<14; i++ ){
    if( btn[i].over ){
      n = i;
      break;
    }
  }

  switch(n){
    case 0:
      f_playing = false;
      time = 0;
      ph1 = 0;
      ph2 = 0;
      phase1 = 0;
      phase2 = 0;
      show_button(1,0);
      enable_button( 4, 0 );
      enable_button( 5, 0 );
      enable_button( 10, 0 );
      enable_button( 11, 0 );
      break;
    case 1:
      if( f_playing ){
        f_playing = false;
        enable_button( 4, 0 );
        enable_button( 5, 0 );
        enable_button( 10, 0 );
        enable_button( 11, 0 );
      } else {
        f_playing = true;
        enable_button( 4, 1 );
        enable_button( 5, 1 );
        enable_button( 10, 1 );
        enable_button( 11, 1 );
      }
      show_button( 1, 1 );
      break;
    case 2:
      len1 --;
      if(len1 <= 1 ){
        len1 = 1;
        enable_button(2,0);
      }
      if( len1 == 34 ){
        enable_button(3,1);
      }
      break;
    case 3:
      len1 ++;
      if( len1 >= 35 ){
        len1 = 35;
        enable_button(3,0);
      }
      if( len1 == 2 ){
        enable_button(2,1);
      }
      break;
    case 4:
      vel1 --;
      if(vel1 <= -16 ){
        vel1 = -16;
        enable_button(4,0);
      }
      if( vel1 == 34 ){
        enable_button(5,1);
      }
      f_vel_changed = true;
      break;
    case 5:
      vel1 ++;
      if( vel1 >= 35 ){
        vel1 = 35;
        enable_button(5,0);
      }
      if( vel1 == -15 ){
        enable_button(4,1);
      }
      f_vel_changed = true;
      break;
    case 6:
      amp1 --;
      if(amp1 <= 0 ){
        amp1 = 0;
        enable_button(6,0);
      }
      if( amp1 == 34 ){
        enable_button(7,1);
      }
      break;
    case 7:
      amp1 ++;
      if( amp1 >= 35 ){
        amp1 = 35;
        enable_button(7,0);
      }
      if( amp1 == 1 ){
        enable_button(6,1);
      }
      break;
    case 8:
      len2 --;
      if(len2 <= 1 ){
        len2 = 1;
        enable_button(8,0);
      }
      if( len2 == 34 ){
        enable_button(9,1);
      }
      break;
    case 9:
      len2 ++;
      if( len2 >= 35 ){
        len2 = 35;
        enable_button(9,0);
      }
      if( len2 == 2 ){
        enable_button(8,1);
      }
      break;
    case 10:
      vel2 --;
      if(vel2 <= -16 ){
        vel2 = -16;
        enable_button(10,0);
      }
      if( vel2 == 34 ){
        enable_button(11,1);
      }
      f_vel_changed = true;
      break;
    case 11:
      vel2 ++;
      if( vel2 >= 35 ){
        vel2 = 35;
        enable_button(11,0);
      }
      if( vel2 == -15 ){
        enable_button(10,1);
      }
      f_vel_changed = true;
      break;
    case 12:
      amp2 --;
      if(amp2 <= 0 ){
        amp2 = 0;
        enable_button(12,0);
      }
      if( amp2 == 34 ){
        enable_button(13,1);
      }
      break;
    case 13:
      amp2 ++;
      if( amp2 >= 35 ){
        amp2 = 35;
        enable_button(13,0);
      }
      if( amp2 == 1 ){
        enable_button(12,1);
      }
      break;
  }

  if( f_playing == false ){
    show_waves();
  }

}



// ========================= ボタン制御 ============================

/////////////////////////////////
// ボタン登録
/////////////////////////////////
function regist_buttons() {

  var y = top_margin + graph_height + gap_graph_panel + 60;

  var x1 = 290;
  var x2 = x1 + btn_size_x * 2 + gap_x + gap_waves;

  btn[0] = new button( panel_side + 40, y + 0, 80, 35, 13);  // リセットボタン
  btn[1] = new button( panel_side + 40, y + 90, 80, 35, 13); // 再生・停止ボタン

  // wave1用

  // 波長
  btn[2] = new button(x1, y, btn_size_x, btn_size_y, 5);
  btn[3] = new button(x1 + btn_size_x + gap_x, y, btn_size_x, btn_size_y, 5);
  // 速度
  btn[4] = new button(x1, y + gap_y + btn_size_y, btn_size_x, btn_size_y, 5);
  btn[5] = new button(x1 + btn_size_x + gap_x, y + gap_y + btn_size_y, btn_size_x, btn_size_y, 5);
  // 振幅
  btn[6] = new button(x1, y + (gap_y + btn_size_y) * 2, btn_size_x, btn_size_y, 5);
  btn[7] = new button(x1 + btn_size_x + gap_x, y + (gap_y + btn_size_y) * 2, btn_size_x, btn_size_y, 5);

  // wave2用

  // 波長
  btn[8] = new button(x2, y, btn_size_x, btn_size_y, 5);
  btn[9] = new button(x2 + btn_size_x + gap_x, y, btn_size_x, btn_size_y, 5);
  // 速度
  btn[10] = new button(x2, y + gap_y + btn_size_y, btn_size_x, btn_size_y, 5);
  btn[11] = new button(x2 + btn_size_x + gap_x, y + gap_y + btn_size_y, btn_size_x, btn_size_y, 5);
  // 振幅
  btn[12] = new button(x2, y + (gap_y + btn_size_y) * 2, btn_size_x, btn_size_y, 5);
  btn[13] = new button(x2 + btn_size_x + gap_x, y + (gap_y + btn_size_y) * 2, btn_size_x, btn_size_y, 5);

}

////////////////////////////////
// ボタンの構造体
////////////////////////////////
function button( x, y, w, h, c ){
  this.x = x;
  this.y = y;
  this.w = w;
  this.h = h;
  this.c = c;
  this.enable = true;
  this.over = false;
}

////////////////////////////////
// ボタン確認（変化があったボタンだけ書き換える）
////////////////////////////////
function check_buttons () {

  var i;

  for( i=0; i<14; i++ ){

    if( btn[i].enable == false ){
      show_button(i,-1);
      btn[i].over = false;
    } else if( btn[i].over ){
      if( !is_over_button(i) ){
        show_button(i,0);
        btn[i].over = false;
      }
    } else {
      if( is_over_button(i) ){
        show_button(i,1);
        btn[i].over = true;
      }
    }

  }

}

////////////////////////////////
// 全ボタン作成（全てのボタンを書き直す）
////////////////////////////////
function show_all_buttons() {

  var i;
  var x = panel_side;
  var y = top_margin + graph_height + gap_graph_panel;

  ctx.font = "16px 'monospace'";
  ctx.fillText("波長", panel_side + 200, y + 80 + 0 );
  ctx.fillText("速さ", panel_side + 200, y + 80 + 70);
  ctx.fillText("振幅", panel_side + 200, y + 80 + 140 );

  ctx.font = "bold 18px 'monospace'";
  ctx.fillText("wave1", 310, y + 30);
  ctx.fillText("wave2", 470, y + 30);

  for( i=0; i<14; i++ ){
    if( !btn[i].enable ){
      show_button(-1);
      btn[i].over = false;
    } else if( is_over_button(i) ){
      show_button(i,1);
      btn[i].over = true;
    } else {
      show_button(i,0);
      btn[i].over = false;
    }
  }

}


////////////////////////////////
// ボタン作成（f; 1:光る 0:通常 -1:無効)
////////////////////////////////
function show_button( n, f){

  var x = btn[n].x;
  var y = btn[n].y;
  var w = btn[n].w;
  var h = btn[n].h;
  var c = btn[n].c;

  ctx.fillStyle = "rgb(150, 120, 120)";
  fillRoundRect(x-3, y-3, w+6, h+6, c );
  if( f==1 ){
    ctx.fillStyle = "rgb(180, 230, 180)"; // 光る
  } else if( f==0 ){
    ctx.fillStyle = "rgb(125, 180, 90)";
  } else {
    ctx.fillStyle = "rgb(50, 80, 50)"; // 無効
  }
  fillRoundRect(x, y, w, h, c-3 );

  var str;

  if( f==-1 ){
    ctx.fillStyle = "rgb(100, 100, 100)"; // 無効
  } else {
    ctx.fillStyle = "rgb(255, 255, 255)";
  }

  switch(n){
    case 0:
      ctx.font = "bold 16px 'monospace'";
      str = "RESET";
      ctx.fillText(str, x+10, y+23);
      break;
    case 1:
      if( f_playing ){
        ctx.font = "bold 16px 'monospace'";
        str = "■";
        ctx.fillText(str, x+32, y+23);
      } else {
        // 再生を表す▲マーク
        ctx.beginPath();
        ctx.moveTo( x + 35, 3 + y + 7 );
        ctx.lineTo( x + 35, 3 + y + 22 );
        ctx.lineTo( x + 47, 3 + y + 14 );
        ctx.closePath();
        ctx.fill();
      }
      break;
    case 2:
    case 4:
    case 6:
    case 8:
    case 10:
    case 12:
      ctx.font = "bold 16px 'monospace'";
      str = "-";
      ctx.fillText(str, x+15, y+24);
      break;
    case 3:
    case 5:
    case 7:
    case 9:
    case 11:
    case 13:
      ctx.font = "bold 16px 'monospace'";
      str = "+";
      ctx.fillText(str, x+13, y+24);
      break;
  }

}

////////////////////////////////
// ボタンの上にあるか
////////////////////////////////
function is_over_button( n ){

  var x = btn[n].x;
  var y = btn[n].y;
  var w = btn[n].w;
  var h = btn[n].h;

  if( mouseX>x && mouseX<x+w && mouseY>y && mouseY<y+h ){
    return true;
  }
  return false;
}


////////////////////////////////
// ボタン有効・無効
////////////////////////////////
function enable_button(n,f){

  if( f ){
    btn[n].enable = true;
    if( is_over_button(n) ){
      show_button(n,1);
    } else {
      show_button(n,0);
    }
  } else {
    btn[n].enable = false;
    show_button(n,-1);
  }

}



