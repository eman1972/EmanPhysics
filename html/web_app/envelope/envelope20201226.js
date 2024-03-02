///////////////////////////////////////////////////////////////
// 包絡線を描く
// Copyright (c) 2013 K.Hiroe
// 修正 2020/12/20

// 注意：素人が見様見真似で作ったものなので参考にすべきでない。


//////////////////////////
// 大域変数
//////////////////////////
var width = 480;
var height = 360;
var scale = 60;    // 1目盛のピクセル数
var ctx;           // canvasコンテキスト
var PI;
var timerID;

var BTN_RET_X;
var BTN_RET_Y;

var BTN_X = [];
var BTN_Y = [];

var cnt_max = [];

var mouseX, mouseY;
var f_over_reset;
var f_over_btn = [];

var mode;
var counter;
var graph_no;
var btn_num;


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

  // マウスイベント関数の設定
  canvas.addEventListener('mousemove', updateMousePos, false);
  canvas.addEventListener('mouseout', resetMousePos, false);
  canvas.addEventListener('click', onClick, false);

  // 画像を読み込む
  img_ttl = new Image();
  img_f0 = new Image();
  img_f1 = new Image();
  img_f2 = new Image();
  img_f3 = new Image();
  img_f4 = new Image();
  img_ttl.src = "./envelope/title.png";
  img_f0.src = "./envelope/img0.png";
  img_f1.src = "./envelope/img1.png";
  img_f2.src = "./envelope/img2.png";
  img_f3.src = "./envelope/img3.png";
  img_f4.src = "./envelope/img4.png";

  // 画像データの読み込み終了待ちインターバルタイマー
  timerID = setInterval('inittimerfunc()',100);
}


/////////////////////////////////////
// 画像の読み込みを待って初期化する
/////////////////////////////////////
function inittimerfunc() {
  try {
    // 画像のサイズをチェック
    if (img_ttl.width == 0) return;
    if (img_f0.width == 0) return;
    if (img_f1.width == 0) return;
    if (img_f2.width == 0) return;
    if (img_f3.width == 0) return;
    if (img_f4.width == 0) return;

    // 初期化の続き
    clearInterval(timerID);
    init_second();
  } catch (e) {
    // 読み込みはまだ完了していない
  }
}

/////////////////////////////////////
// 画像の読み込みの後で初期化の続き
/////////////////////////////////////
function init_second() {

  menu_start();

  // タイマーイベント関数の設定
  timerID = setInterval('timerfunc()', 200);  // 秒間30フレーム

}

/////////////////////////////////
// 初期化
/////////////////////////////////
function init_vars() {

  PI = Math.PI;
  btn_num = 5;

  BTN_RET_X = 390;
  BTN_RET_Y = 320;

  BTN_X[0] = 30;
  BTN_Y[0] = 65;

  BTN_X[1] = 30;
  BTN_Y[1] = 125;

  BTN_X[2] = 30;
  BTN_Y[2] = 185;

  BTN_X[3] = 30;
  BTN_Y[3] = 245;

  BTN_X[4] = 30;
  BTN_Y[4] = 305;

  cnt_max[0] = 30;
  cnt_max[1] = 30;
  cnt_max[2] = 30;
  cnt_max[3] = 15;
  cnt_max[4] = 30;

  mouseX = 100;
  mouseY = 100;

  f_over_reset = false;

  for(i=0; i<btn_num; i++){
    f_over_btn[i] = false;
  }

  mode = 0;
  counter = 0;
  graph_no = 0;
}

/////////////////////////////////
// メニュー画面に戻す
/////////////////////////////////
function menu_start() {

  mode = 0;

  // 画面消去
  ctx.fillStyle = "rgb(255,255,255)";
  ctx.fillRect(0, 0, width, height);

  // 外枠表示
  ctx.strokeStyle = "rgb(0,0,0)";
  ctx.lineCap = "square";
  ctx.strokeRect(0,0,width,height);

  var i;
  for(i=0; i<btn_num; i++ ){
    if( is_over_btn[i] ){
      show_button(i,1);
    } else {
      show_button(i,0);
    }
  }

  ctx.drawImage(img_ttl, 30, 15);
  ctx.drawImage(img_f0, BTN_X[0]+50, BTN_Y[0]);
  ctx.drawImage(img_f1, BTN_X[1]+50, BTN_Y[1]-10);
  ctx.drawImage(img_f2, BTN_X[2]+50, BTN_Y[2]-10);
  ctx.drawImage(img_f3, BTN_X[3]+50, BTN_Y[3]+1);
  ctx.drawImage(img_f4, BTN_X[4]+50, BTN_Y[4]+5);

}


/////////////////////////////////
// グラフ描画を開始する
/////////////////////////////////
function graph_start() {

  // 画面消去
  ctx.fillStyle = "rgb(255,255,255)";
  ctx.fillRect(0, 0, width, height);


  // 座標軸の描画
  draw_coordinates();

  // 外枠表示
  ctx.strokeStyle = "rgb(0,0,0)";
  ctx.lineCap = "square";
  ctx.strokeRect(0,0,width,height);

  f_over_reset = false;
  show_reset_button(0);

  counter = 0;

}

/////////////////////////////////
// 新しい線を一本追加する
/////////////////////////////////
function add_new_line() {

  if( graph_no == 3 ){
    x = Math.cos( counter/cnt_max[graph_no] * PI/2 );
    y = Math.sin( counter/cnt_max[graph_no] * PI/2 );
  } else {
    x = Math.cos( counter/cnt_max[graph_no] * PI );
    y = Math.sin( counter/cnt_max[graph_no] * PI );
  }
  c = y/x;

  switch(graph_no){
    case 0: seppen = c*c; break;
    case 1: seppen = 1/c; break;
    case 2: seppen = 1/(c-1); break;
    case 3: seppen = Math.sqrt(c); break;
    case 4: seppen = c; break;
  }


  draw_line( x, y, seppen );

  // 最後にリセットボタンを上書き
  if( f_over_reset ){
    show_reset_button(1);
  } else {
    show_reset_button(0);
  }

}

/////////////////////////////////
// 包絡線を追加する
/////////////////////////////////
function add_envelope() {

  switch(graph_no){
    case 0: draw_curve0(); break;
    case 1: draw_curve1(); break;
    case 2: draw_curve2(); break;
    case 3: draw_curve3(); break;
    case 4: break;
  }

  // 最後にリセットボタンを上書き
  if( f_over_reset ){
    show_reset_button(1);
  } else {
    show_reset_button(0);
  }

}

/////////////////////////////////
// 画面の描画
/////////////////////////////////
function draw() {

  // 画面消去
  ctx.fillStyle = "rgb(255,255,255)";
  ctx.fillRect(0, 0, width, height);


  // 座標軸の描画
  draw_coordinates();

  // 外枠表示
  ctx.strokeStyle = "rgb(0,0,0)";
  ctx.lineCap = "square";
  ctx.strokeRect(0,0,width,height);

}

/////////////////////////////////
// 傾きy/xで切片cを通る直線を描く
/////////////////////////////////
function draw_line( x, y, c ){

  range = 10;
  f_solved = false;
  f_yaxis = false;    // y 軸交点で決めることになった場合。
  
  // y = ±range との交点を求める。
  if( y != 0 ){
    xplus = x*(range-c)/y;
    xminus = x*(-range-c)/y;

    f_xplus = ( xplus>=-range && xplus<=range );
    f_xminus = ( xminus>=-range && xminus<=range );

    if( f_xplus && f_xminus ){
      // 決定
      nx1 = xplus * scale;
      ny1 = range * scale;

      nx2 = xminus * scale;
      ny2 = -range * scale;

      f_solved = true;

    } else if( f_xplus || f_xminus ){
      // 片方決定
      if( f_xplus ){
        nx1 = xplus * scale;
        ny1 = range * scale;
      } else {
        nx1 = xminus * scale;
        ny1 = -range * scale;
      }

    } else { // ともに範囲外
      if( xplus * xminus >= 0 ){
          // 同符号だから範囲内に直線なし。　終了。
          return;
      } else {
        // 両方別計算で決まる。
        f_yaxis = true;
      }
    }
  }

  // x = ±rangeとの交点を求める。
  if( !f_solved ){
    yplus = y*range/x + c;
    yminus = -y*range/x + c;

    if( f_yaxis ){ // ここで両方決定
      nx1 = range * scale;
      ny1 = yplus * scale;

      nx2 = -range * scale;
      ny2 = yminus * scale;

    } else if( yplus>=-range && yplus<=range ){
      // もう一つの交点はyplusに決定
      nx2 = range * scale;
      ny2 = yplus * scale;
    } else {
      // もう一つの交点はyminusに決定
      nx2 = -range * scale;
      ny2 = yminus * scale;
    }
  }

  // いよいよ描画
  ctx.beginPath();
  ctx.lineWidth = 1;
  ctx.lineCap = "round";
  ctx.strokeStyle = "rgb(10,10,250)";

  if( c<=range && c>=-range ){
    // 切片が範囲内にあれば、切片から両方の点に線を引く
    ctx.moveTo(nx1 + width/2, -ny1 + height/2);
    ctx.lineTo(width/2, height/2 - c*scale);          // ズレを防ぐためy軸との交点を経由
    ctx.lineTo(nx2 + width/2, -ny2 + height/2);

  } else {
    // 両方の点を直接結ぶように線を引く
    ctx.moveTo(nx1 + width/2, -ny1 + height/2);
    ctx.lineTo(nx2 + width/2, -ny2 + height/2);
  }

  ctx.stroke();

}

/////////////////////////////////
// 曲線を描く y = -(1/4)x^2
/////////////////////////////////
function draw_curve0(){

  var x,y;

  // 画面外を指定しても正しく処理してくれるようなので、頼って簡単に済ませる。

  ctx.lineWidth = 2;
  ctx.lineCap = "round";
  ctx.strokeStyle = "rgb(230,10,10)";

  ctx.beginPath();
  x = -4;
  y = -x*x/4;
  ctx.moveTo(x*scale + width/2, -y*scale + height/2);

  for( x=-4; x<4; x+=0.05 ){
    y = -x*x/4;
    ctx.lineTo(x*scale + width/2, -y*scale + height/2);
  }

  ctx.stroke();
}

/////////////////////////////////
// 曲線を描く y = ±2√x
/////////////////////////////////
function draw_curve1(){

  var x,y;

  // 画面外を指定しても正しく処理してくれるようなので、頼って簡単に済ませる。

  ctx.lineWidth = 2;
  ctx.lineCap = "round";
  ctx.strokeStyle = "rgb(230,10,10)";

  ctx.beginPath();
  ctx.moveTo( width/2, height/2 );
  for( x=0; x<4; x+=0.05 ){
    y = 2 * Math.sqrt(x);
    ctx.lineTo(x*scale + width/2, -y*scale + height/2);
  }

  ctx.moveTo( width/2, height/2 );
  for( x=0; x<4; x+=0.05 ){
    y = -2 * Math.sqrt(x);
    ctx.lineTo(x*scale + width/2, -y*scale + height/2);
  }

  ctx.stroke();

}

/////////////////////////////////
// 曲線を描く y = ±2√x + x
/////////////////////////////////
function draw_curve2(){

  var x,y;

  // 画面外を指定しても正しく処理してくれるようなので、頼って簡単に済ませる。

  ctx.lineWidth = 2;
  ctx.lineCap = "round";
  ctx.strokeStyle = "rgb(230,10,10)";

  ctx.beginPath();
  ctx.moveTo( width/2, height/2 );
  for( x=0; x<4; x+=0.05 ){
    y = 2 * Math.sqrt(x) + x;
    ctx.lineTo(x*scale + width/2, -y*scale + height/2);
  }

  ctx.moveTo( width/2, height/2 );
  for( x=0; x<4; x+=0.05 ){
    y = -2 * Math.sqrt(x) + x;
    ctx.lineTo(x*scale + width/2, -y*scale + height/2);
  }

  ctx.stroke();

}

/////////////////////////////////
// 曲線を描く y = 3/(4x)
/////////////////////////////////
function draw_curve3(){

  var x,y;

  // 画面外を指定しても正しく処理してくれるようなので、頼って簡単に済ませる。

  ctx.lineWidth = 2;
  ctx.lineCap = "round";
  ctx.strokeStyle = "rgb(230,10,10)";

  ctx.beginPath();

  ctx.moveTo( -4*scale + width/2, height/2 );
  for( x=-4; x<0; x+=0.05 ){
    y = -1/4/x;
    if( y>10 ) y = 10;
    ctx.lineTo(x*scale + width/2, -y*scale + height/2);
  }

  ctx.stroke();

}

/////////////////////////////////
// 座標軸の描画
/////////////////////////////////
function draw_coordinates(){

  ctx.strokeStyle = "rgb(0,0,0)"; // 線は黒で

  // 縦線表示
  var i;
  ctx.lineWidth = 0.2;

  ctx.beginPath();
  for( i=-3; i<4; i++ ){
    ctx.moveTo(width/2+i*scale,0);
    ctx.lineTo(width/2+i*scale,height);
  }

  // 横線表示
  for( i=-3; i<4; i++ ){
    ctx.moveTo(0,height/2+i*scale);
    ctx.lineTo(width,height/2+i*scale);
  }

  ctx.stroke();

  ctx.lineWidth = 1.0;
  ctx.beginPath();

  // x軸表示
  ctx.moveTo(0,height/2);
  ctx.lineTo(width,height/2);

  // y軸表示
  ctx.moveTo(width/2,0);
  ctx.lineTo(width/2,height);

  ctx.stroke();

  var str = "O";
  ctx.font = "14px 'Times New Roman'";
  ctx.fillStyle = "rgb(0, 0, 0)";
  ctx.fillText(str, width/2 - 13, height/2 + 14);

}

////////////////////////////////
// 戻るボタン作成
////////////////////////////////
function show_reset_button(f) {

  ctx.fillStyle = "rgb(100, 100, 100)";
  fillRoundRect(BTN_RET_X-3, BTN_RET_Y-3, 70+5, 26+6, 13 );
  if(f){
    ctx.fillStyle = "rgb(255, 150, 150)"; // 光る
  } else {
    ctx.fillStyle = "rgb(255, 90, 90)";
  }
  fillRoundRect(BTN_RET_X, BTN_RET_Y, 70, 26, 10 );

  var str;
  ctx.fillStyle = "rgb(0, 255, 255)";
  ctx.font = "bold 18px sans-serif";
  str = "戻る";
  ctx.fillText(str, BTN_RET_X+15, BTN_RET_Y+20);

}

////////////////////////////////
// 選択ボタン作成
////////////////////////////////
function show_button(no,f) {

  ctx.fillStyle = "rgb(100, 100, 100)";
  fillRoundRect(BTN_X[no]-3, BTN_Y[no]-3, 30+6, 30+6, 13 );
  if(f){
    ctx.fillStyle = "rgb(180, 230, 180)"; // 光る
  } else {
    ctx.fillStyle = "rgb(125, 180, 90)";
  }
  fillRoundRect(BTN_X[no], BTN_Y[no], 30, 30, 10 );

//  var str;
//  ctx.fillStyle = "rgb(0, 255, 255)";
//  ctx.font = "bold 16px sans-serif";
//  str = "戻る";
//  ctx.fillText(str, BTN_RET_X+10, BTN_RET_Y+25);

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
  ctx.fill();
}

////////////////////////////////
// 戻るボタンの上にあるか
////////////////////////////////
function is_over_reset(){

  if( mouseX>BTN_RET_X && mouseX<BTN_RET_X+70 && mouseY>BTN_RET_Y && mouseY<BTN_RET_Y+26 ){
    return true;
  }
  return false;
}


////////////////////////////////
// 選択ボタンの上にあるか
////////////////////////////////
function is_over_btn(no){

  if( mouseX>BTN_X[no] && mouseX<BTN_X[no]+30 && mouseY>BTN_Y[no] && mouseY<BTN_Y[no]+30 ){
    return true;
  }
  return false;
}

////////////////////////////////
// タイマーイベント関数
////////////////////////////////
function timerfunc() {

  if( mode != 1 ) return;

  counter ++;
  if( counter >= cnt_max[graph_no] ){
    add_envelope();
    mode = 2;
    return;
  }
  add_new_line();

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

  if( mode >= 1 ){
    if( f_over_reset ){
      f_over_reset = false;
      resetMousePos();
      menu_start();
      check_buttons();
    }
    return;
  }

  var i;
  for(i=0; i<btn_num; i++){
    if( f_over_btn[i] ){
      mode = 1;
      graph_no = i;
      graph_start();
      return;
    }
  }

}

////////////////////////////////
// ボタン確認
////////////////////////////////
function check_buttons () {

  if( mode >= 1 ){
    if( f_over_reset ){
      if( !is_over_reset() ){
        show_reset_button(0);
        f_over_reset = false;
      }
    } else {
      if( is_over_reset() ){
        show_reset_button(1);
        f_over_reset = true;
      }
    }
    return;
  }

  var i;

  for(i=0; i<btn_num; i++ ){
    if( f_over_btn[i] ){
      if( !is_over_btn(i) ){
        show_button(i,0);
        f_over_btn[i] = false;
      }
    } else {
      if( is_over_btn(i) ){
        show_button(i,1);
        f_over_btn[i] = true;
      }
    }
  }

}

