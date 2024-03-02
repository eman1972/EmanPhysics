///////////////////////////////////////////////////////////////
// エネルギー反射率のグラフ
// Copyright (c) 2014 K.Hiroe

// 注意：素人が見様見真似で作ったものなので参考にすべきでない。


//////////////////////////
// 大域変数
//////////////////////////
var width = 480;
var height = 500;

var org_x = 50;
var org_y = 280
var graph_width = 400;
var graph_height = 250;
var pnum = 180;  // 形状データの点の個数

var ctx;           // canvasコンテキスト
var timerID;
var PIdiv2;

var n1 = 1;
var n2 = 1.8;

var mu1 = 1;
var mu2 = 1;

var mouseX, mouseY;
var f_press = false;
var press_no;


var graph1 = [];
var graph2 = [];

var time = 0;

var btn = [];


// ボタン類の表示位置

// 全体の位置（左上のボタン位置を基準とする）
var console_x = 75;
var console_y = 360;

var btn_size_x = 40;
var btn_size_y = 40;

// パラメータ表示窓の大きさ
var prmbox_x = 50;
var prmbox_y = 32;

var prmbox_y_adj = 3;

var gap_btn_box = 10;  // ボタンとパラメータ表示ボックスの隙間サイズ
var gap_ref_perm = 65; // 屈折率と透磁率の隙間サイズ
var gap_in_trans = 30; // 入射側と透過側の隙間サイズ



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

  // 画面クリア
  ctx.fillStyle="rgb(255,255,255)";
  ctx.fillRect(0,0,width,height);
  ctx.fillStyle="rgb(0,0,0)";
  ctx.lineCap = "square";

/*
  ctx.lineWidth = 1;
  ctx.strokeRect(0,0,width,height);
  ctx.stroke();
*/

  draw_coordinates();
  draw_graphs();
  show_numbers();

  regist_buttons();
  show_all_buttons();

  // マウスイベント関数の設定
  canvas.addEventListener('mousemove', updateMousePos, false);
  canvas.addEventListener('mouseout', resetMousePos, false);
  canvas.addEventListener('mousedown', onMouseDown, false);
  canvas.addEventListener('mouseup', onMouseUp, false);

  // タッチイベント関数の設定
  canvas.addEventListener('touchstart', onTouchStart, false);
  canvas.addEventListener('touchend', onTouchEnd, false);
  canvas.addEventListener('touchmove', onTouchMove, false);

  // タイマーイベント関数の設定
  timerID = setInterval('timerfunc()', 100);  // 秒間10フレーム

}

/////////////////////////////////
// 初期化
/////////////////////////////////
function init_vars() {

  mouseX = 100;
  mouseY = 100;
  f_press = false;

  PI = Math.PI;
  PIdiv2 = Math.PI / 2;

}


////////////////////////////////
// グラフ枠の表示
////////////////////////////////
function draw_coordinates(){

  ctx.fillStyle="rgb(255,255,255)";
  ctx.fillRect(1,1,width-2,org_y+25);

  // 文字
  ctx.fillStyle = "rgb(0, 0, 0)";
  ctx.font = "14px 'monospace'";

  ctx.fillText("S偏光", 60, 60);
  ctx.fillText("P偏光", 60, 90);

  // 凡例
  ctx.lineWidth = 2;
  ctx.lineCap = "round";

  // S偏光ライン
  ctx.beginPath();
  ctx.strokeStyle = "rgb(0,0,255)";
  ctx.moveTo( 105, 55 );
  ctx.lineTo( 155, 55 );
  ctx.stroke();

  // P偏光ライン
  ctx.beginPath();
  ctx.strokeStyle = "rgb(255,0,0)";
  ctx.moveTo( 105, 85 );
  ctx.lineTo( 155, 85 );
  ctx.stroke();

  ctx.strokeStyle = "rgb(0,0,0)";
  ctx.lineWidth = 1.2;
  ctx.lineCap = "square";

  ctx.strokeRect( org_x, org_y, graph_width, -graph_height );

  ctx.beginPath();

  // 縦軸目盛り
  ctx.moveTo( org_x-2, org_y - graph_height/2 );
  ctx.lineTo( org_x+2, org_y - graph_height/2 );
  ctx.fillText("0\%", org_x - 35, org_y + 5 );
  ctx.fillText("50\%", org_x - 40, org_y - graph_height/2 + 5 );
  ctx.fillText("100\%", org_x - 45, org_y - graph_height + 5 );

  // 横軸目盛り
  ctx.moveTo( org_x + graph_width/3, org_y-3 );
  ctx.lineTo( org_x + graph_width/3, org_y+3 );
  ctx.moveTo( org_x + graph_width/3*2, org_y-3 );
  ctx.lineTo( org_x + graph_width/3*2, org_y+3 );
  ctx.fillText("30°", org_x + graph_width/3 - 8, org_y + 20 );
  ctx.fillText("60°", org_x + graph_width/3*2 - 8, org_y + 20 );
  ctx.fillText("90°", org_x + graph_width - 8, org_y + 20 );

  ctx.stroke();


}

function draw_graphs() {

  var i;
  var theta1, theta2;
  var cos1, cos2, sin2;
  var r_s, r_p;

  // 波の線の設定
  ctx.lineWidth = 2;
  ctx.lineCap = "round";

  // 波の形の計算
  for( i=0; i<=pnum; i++ ){
    theta1 = i * PIdiv2/pnum;
    sin2 = Math.sin(theta1) * n1/n2;
    if( sin2 >1 ){
      graph1[i] = 1;
      graph2[i] = 1;
    } else {
      theta2 = Math.asin( sin2 );
      cos1 = Math.cos( theta1 );
      cos2 = Math.cos( theta2 );
      r_s = (mu2 * n1 * cos1 - mu1 * n2 * cos2)/(mu2 * n1 * cos1 + mu1 * n2 * cos2);
      r_p = (mu2 * n1 * cos2 - mu1 * n2 * cos1)/(mu2 * n1 * cos2 + mu1 * n2 * cos1);
      graph1[i] = r_s * r_s;
      graph2[i] = r_p * r_p;
    }
  }

  ctx.beginPath();
  ctx.strokeStyle = "rgb(0,0,255)";
  ctx.moveTo( org_x, -graph1[0] * graph_height + org_y );
  for( i=1; i<=pnum; i++ ){
    ctx.lineTo( org_x + i * graph_width/pnum, -graph1[i] * graph_height + org_y );
  }
  ctx.stroke();

  ctx.beginPath();
  ctx.strokeStyle = "rgb(255,0,0)";
  ctx.moveTo( org_x, -graph2[0] * graph_height + org_y );
  for( i=1; i<=pnum; i++ ){
    ctx.lineTo( org_x + i * graph_width/pnum, -graph2[i] * graph_height + org_y );
  }
  ctx.stroke();

}

// 数値情報表示
function show_numbers() {

  ctx.fillStyle = "rgb(255,255,255)";
  ctx.fillRect( console_x + btn_size_x + gap_btn_box + 1, console_y + prmbox_y_adj + 1, prmbox_x - 2, prmbox_y - 2 );
  ctx.fillRect( console_x + btn_size_x + gap_btn_box + 1, console_y + btn_size_y + gap_in_trans + prmbox_y_adj + 1, prmbox_x - 2, prmbox_y - 2 );
  ctx.fillRect( console_x + btn_size_x * 3 + prmbox_x + gap_btn_box * 3 + gap_ref_perm + 1, console_y + prmbox_y_adj + 1, prmbox_x - 2, prmbox_y - 2 );
  ctx.fillRect( console_x + btn_size_x * 3 + prmbox_x + gap_btn_box * 3 + gap_ref_perm + 1, console_y + btn_size_y + gap_in_trans + prmbox_y_adj + 1, prmbox_x - 2, prmbox_y - 2 );

  ctx.fillStyle = "rgb(0, 0, 0)";
  ctx.font = "15px 'monospace'";

  str = "" + n1.toFixed(2);
  ctx.fillText( str, console_x + btn_size_x + gap_btn_box + 7, console_y + prmbox_y + prmbox_y_adj - 10 );

  str = "" + n2.toFixed(2);
  ctx.fillText( str, console_x + btn_size_x + gap_btn_box + 7, console_y + btn_size_y + gap_in_trans + prmbox_y + prmbox_y_adj - 10 );

  str = "" + mu1.toFixed(2);
  ctx.fillText( str, console_x + btn_size_x * 3 + prmbox_x + gap_btn_box * 3 + gap_ref_perm + 7, console_y + prmbox_y + prmbox_y_adj - 10 );

  str = "" + mu2.toFixed(2);
  ctx.fillText( str, console_x + btn_size_x * 3 + prmbox_x + gap_btn_box * 3 + gap_ref_perm + 7, console_y + btn_size_y + gap_in_trans + prmbox_y + prmbox_y_adj - 10 );

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

  if(f_press){
    if( counter > 5 ){
      changeValues(press_no);
    } else {
      counter ++;
    }
  } else {
    counter = 0;
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
function onMouseDown(event) {

  n = -1;

  // どのボタンの上で押されたかを調べる
  var i;
  for( i=0; i<8; i++ ){
    if( btn[i].over ){
      n = i;
      break;
    }
  }

  f_press = true;
  press_no = n;

  changeValues(n);
}


function changeValues(n){

  switch(n){
    case 0:
      n1 -= 0.01;
      if( n1<=0 ){ n1 = 0; }
      break;
    case 1:
      n1 += 0.01;
      if( n1>=9.99 ){ n1 = 9.99; }
      break;
    case 2:
      n2 -= 0.01;
      if( n2<=0 ){ n2 = 0; }
      break;
    case 3:
      n2 += 0.01;
      if( n2>=9.99 ){ n2 = 9.99; }
      break;
    case 4:
      mu1 -= 0.01;
      if( mu1<=0 ){ mu1 = 0; }
      break;
    case 5:
      mu1 += 0.01;
      if( mu1>=9.99 ){ mu1 = 9.99; }
      break;
    case 6:
      mu2 -= 0.01;
      if( mu2<=0 ){ mu2 = 0; }
      break;
    case 7:
      mu2 += 0.01;
      if( mu2>=9.99 ){ mu2 = 9.99; }
      break;
  }

  draw_coordinates();
  draw_graphs();
  show_numbers();
}

////////////////////////////////
// マウスボタンが離されたときの処理
////////////////////////////////
function onMouseUp(event) {

  f_press = false;
}

////////////////////////////////
// 指が画面に触れたときの処理
////////////////////////////////
function onTouchStart(event) {

  var rect = event.target.getBoundingClientRect();

  mouseX = event.changedTouches[0].pageX - rect.left - window.pageXOffset;
  mouseY = event.changedTouches[0].pageY - rect.top - window.pageYOffset;;

  check_buttons2();

  event.preventDefault(); // 同時発生するマウスイベントを消す

}

////////////////////////////////
// 指が画面上で動いたときの処理
////////////////////////////////
function onTouchMove(event) {

  var rect = event.target.getBoundingClientRect();

  mouseX = event.changedTouches[0].pageX - rect.left - window.pageXOffset;
  mouseY = event.changedTouches[0].pageY - rect.top - window.pageYOffset;;

  check_buttons2();

}

////////////////////////////////
// 指が画面から離されたときの処理
////////////////////////////////
function onTouchEnd(event) {

  mouseX = -100;
  mouseY = -100;

  check_buttons2();

  f_press = false;
}


// ========================= ボタン制御 ============================

/////////////////////////////////
// ボタン登録
/////////////////////////////////
function regist_buttons() {

  // 屈折率1
  btn[0] = new button(console_x, console_y, btn_size_x, btn_size_y, 5);
  btn[1] = new button(console_x + prmbox_x + btn_size_x + gap_btn_box * 2, console_y, btn_size_x, btn_size_y, 5);
  // 屈折率2
  btn[2] = new button(console_x, console_y + btn_size_y + gap_in_trans, btn_size_x, btn_size_y, 5);
  btn[3] = new button(console_x + prmbox_x + btn_size_x + gap_btn_box * 2, console_y + btn_size_y + gap_in_trans, btn_size_x, btn_size_y, 5);
  // 透磁率1
  btn[4] = new button(console_x + btn_size_x * 2 + prmbox_x + gap_btn_box * 2 + gap_ref_perm, console_y, btn_size_x, btn_size_y, 5);
  btn[5] = new button(console_x + btn_size_x * 3 + prmbox_x * 2 + gap_btn_box * 4 + gap_ref_perm, console_y, btn_size_x, btn_size_y, 5);
  // 透磁率2
  btn[6] = new button(console_x + btn_size_x * 2 + prmbox_x + gap_btn_box * 2 + gap_ref_perm, console_y + btn_size_y + gap_in_trans, btn_size_x, btn_size_y, 5);
  btn[7] = new button(console_x + btn_size_x * 3 + prmbox_x * 2 + gap_btn_box * 4 + gap_ref_perm, console_y + btn_size_y + gap_in_trans, btn_size_x, btn_size_y, 5);

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

  for( i=0; i<8; i++ ){

    if( btn[i].enable == false ){
      show_button(i,-1);
      btn[i].over = false;
    } else if( btn[i].over ){
      if( !is_over_button(i) ){
        f_press = false;
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
// ボタン確認（タップ操作用）
////////////////////////////////
function check_buttons2() {

  var i;

  for( i=0; i<8; i++ ){

    if( btn[i].enable == false ){
      show_button(i,-1);
      btn[i].over = false;
    } else if( btn[i].over ){
      if( !is_over_button(i) ){
        f_press = false;
        show_button(i,0);
        btn[i].over = false;
      }
    } else {
      if( is_over_button(i) ){
        show_button(i,1);
        btn[i].over = true;
        f_press = true;
        press_no = i;
        changeValues(i);
      }
    }
  }
}

////////////////////////////////
// 全ボタン作成（全てのボタンを書き直す）
////////////////////////////////
function show_all_buttons() {

  var i;

  ctx.font = "15px 'monospace'";
  ctx.fillText("屈折率", console_x + btn_size_x + gap_btn_box + 1, console_y - 20);
  ctx.fillText("比透磁率", console_x + btn_size_x * 3 + prmbox_x + gap_btn_box * 3 + gap_ref_perm - 8, console_y - 20);
  ctx.fillText("入射側", console_x - 60, console_y + 25 );
  ctx.fillText("透過側", console_x - 60, console_y + btn_size_y + gap_in_trans + 25 );

  ctx.strokeStyle = "rgb(0,0,0)";
  ctx.lineWidth = 1;
  ctx.lineCap = "square";
  ctx.strokeRect( console_x + btn_size_x + gap_btn_box, console_y + prmbox_y_adj, prmbox_x, prmbox_y );
  ctx.strokeRect( console_x + btn_size_x + gap_btn_box, console_y + btn_size_y + gap_in_trans + prmbox_y_adj, prmbox_x, prmbox_y );
  ctx.strokeRect( console_x + btn_size_x * 3 + prmbox_x + gap_btn_box * 3 + gap_ref_perm, console_y + prmbox_y_adj, prmbox_x, prmbox_y );
  ctx.strokeRect( console_x + btn_size_x * 3 + prmbox_x + gap_btn_box * 3 + gap_ref_perm, console_y + btn_size_y + gap_in_trans + prmbox_y_adj, prmbox_x, prmbox_y );


  for( i=0; i<8; i++ ){
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
    case 2:
    case 4:
    case 6:
      ctx.font = "bold 18px 'monospace'";
      str = "-";
      ctx.fillText(str, x + btn_size_x/2 - 5, y + btn_size_y/2 + 5);
      break;
    case 1:
    case 3:
    case 5:
    case 7:
      ctx.font = "bold 16px 'monospace'";
      str = "+";
      ctx.fillText(str, x + btn_size_x/2 - 7, y + btn_size_y/2 + 3);
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



