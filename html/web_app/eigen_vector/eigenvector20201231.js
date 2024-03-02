///////////////////////////////////////////////////////////////
// 固有ベクトル
// Copyright (c) 2011 K.Hiroe

// 注意：素人が見様見真似で作ったものなので参考にすべきでない。


//////////////////////////
// 大域変数
//////////////////////////
var width = 640;
var height = 480;
var scale = 80;    // 1目盛のピクセル数
var dif_angle = 1.5;
var dif_radius = 0.015;
var near0 = 0.0001; // 絶対値がこれ以下はもう0と看做そうよ
var ctx;           // canvasコンテキスト
var timerID;
var key_space;
var key_angle_up;
var key_angle_down;
var key_radius_up;
var key_radius_down;
var key_sample;
var key_angle_up_y;
var key_angle_down_y;
var key_radius_up_y;
var key_radius_down_y;
var key_sample_mode;
var angle_x;
var angle_y;
var angle_s;
var radius_x;
var radius_y;
var radius_s;
var sample_mode;

var mat_a;
var mat_b;
var mat_c;
var mat_d;
var lambda1;
var lambda2;
var imaginary;
var vec_num;
var f_draw_all;
var eigen_vec = new Array(2);
var img_kakko;

var btn = [];
var mouseX, mouseY;
var press_no;

var canvas;

var btn_x_angle_up;
var btn_x_angle_down;
var btn_x_radius_up;
var btn_x_radius_down;
var btn_y_angle_up;
var btn_y_angle_down;
var btn_y_radius_up;
var btn_y_radius_down;
var btn_s_angle_up;
var btn_s_angle_down;
var btn_s_radius_up;
var btn_s_radius_down;
var btn_reset;
var btn_x;

//////////////////////////
// 初期化
//////////////////////////
onload = function() {

  // 画面表示用コンテキストの取得
  canvas = document.getElementById('drawfield');
  if (!canvas || !canvas.getContext) {
    alert("HTML5対応ブラウザでのみお試し頂けます");
    return false;
  }

  ctx = canvas.getContext('2d');

  // 大域変数の初期化
  init_vars();

  // 画像を読み込む
  img_kakko = new Image();
  img_kakko.src = "kakko.png";

  // 画像データの読み込み終了待ちインターバルタイマー
  timerID = setInterval('inittimerfunc()',100);
}


/////////////////////////////////////
// 画像の読み込みを待って初期化する
/////////////////////////////////////
function inittimerfunc() {
  try {
    // 画像のサイズをチェック
    if (img_kakko.width == 0) {
      return;
    }
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

  ctx.fillStyle="rgb(255,255,255)";
  ctx.fillRect(0,0,width,height);

  draw();
  regist_buttons();

  // サンプルベクトル操作系は最初は無効にしておく
  enable_sample_buttons(false);

  show_all_buttons();

  // キーボードイベント関数の設定
  window.addEventListener('keydown',keydownfunc,true);
  window.addEventListener('keyup',keyupfunc,true);

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
  timerID = setInterval('timerfunc()', 33);  // 秒間30フレーム

}

/////////////////////////////////
// 初期化
/////////////////////////////////
function init_vars() {

  mouseX = 100;
  mouseY = 100;

  key_space = 0;
  key_angle_up = 0;
  key_angle_down = 0;
  key_radius_up = 0;
  key_radius_down = 0;
  key_sample = 0;
  key_angle_up_y = 0;
  key_angle_down_y = 0;
  key_radius_up_y = 0;
  key_radius_down_y = 0;
  key_reset = 0;
  key_sample_mode = 0;
  sample_mode = false;
  vec_num = 0;

  btn_x_angle_up = 0;
  btn_x_angle_down = 0;
  btn_x_radius_up = 0;
  btn_x_radius_down = 0;
  btn_y_angle_up = 0;
  btn_y_angle_down = 0;
  btn_y_radius_up = 0;
  btn_y_radius_down = 0;
  btn_s_angle_up = 0;
  btn_s_angle_down = 0;
  btn_s_radius_up = 0;
  btn_s_radius_down = 0;
  btn_reset = 0;
  btn_x = 0;
  press_no = -1;

  angle_x = 0;
  angle_y = 90;
  angle_s = 15;
  radius_x = 1.0;
  radius_y = 1.0;
  radius_s = 1.0;

  PI = Math.PI;

}

/////////////////////////////////
// 画面の描画
/////////////////////////////////
function draw() {

  // 描画に必要な数値を出す
  calculate();

  // 画面消去
  ctx.fillStyle = "rgb(255,255,255)";
  ctx.fillRect(0, 0, width, height);

  // 固有ベクトルが任意である場合
  if( f_draw_all == 1 ){
    draw_all_direction();
  }

  // 座標軸の描画
  draw_coordinates();

  // 平行四辺形表示
  draw_parallelograms();

  // 固有ベクトル表示
  if( f_draw_all == 0 ){
    if( vec_num == 1 ){
      draw_line( eigen_vec[0].x, eigen_vec[0].y );
    } else if( vec_num == 2 ){
      draw_line( eigen_vec[0].x, eigen_vec[0].y );
      draw_line( eigen_vec[1].x, eigen_vec[1].y );
    }
  }

  // 基底ベクトル表示
  draw_vector( mat_a, mat_c,"rgb(0,200,50)" );
  draw_vector( mat_b, mat_d,"rgb(0,0,255)" );

  // サンプルベクトルと変換後ベクトルの表示
  if( sample_mode ){
    draw_samples();
  }

  // 数値情報表示
  disp_numbers();

  // 外枠表示
  ctx.strokeStyle = "rgb(0,0,0)";
  ctx.lineCap = "square";
  ctx.strokeRect(0,0,width,height);

}

////////////////////////////////
// 計算
////////////////////////////////
function calculate() {

  f_draw_all = 0;

  mat_a = radius_x * Math.cos( angle_x/360*2*Math.PI );
  mat_c = radius_x * Math.sin( angle_x/360*2*Math.PI );
  mat_b = radius_y * Math.cos( angle_y/360*2*Math.PI );
  mat_d = radius_y * Math.sin( angle_y/360*2*Math.PI );

  var discriminant = (mat_a - mat_d)*(mat_a - mat_d) + 4 * mat_b * mat_c;
  if( discriminant < -near0 ){
    // 固有値を複素表現する。固有ベクトルの計算はしない。
    vec_num = 0;
    lambda1 = (mat_a + mat_d)/2;
    imaginary = Math.sqrt(-discriminant)/2;
  } else if( discriminant < near0 ){
    // 固有値を一つだけ書く。固有ベクトルを計算する。
    vec_num = 1;
    lambda1 = (mat_a + mat_d)/2;
    decide_eigen_vector( mat_a, mat_b, mat_c, mat_d, lambda1, 0 );
  } else {
    vec_num = 2;
    lambda1 = (mat_a + mat_d + Math.sqrt(discriminant)) / 2
    lambda2 = (mat_a + mat_d - Math.sqrt(discriminant)) / 2
    decide_eigen_vector( mat_a, mat_b, mat_c, mat_d, lambda1, 0 );
    decide_eigen_vector( mat_a, mat_b, mat_c, mat_d, lambda2, 1 );
  }
}

////////////////////////////////
// 固有ベクトルを決定する
////////////////////////////////
function decide_eigen_vector(a,b,c,d,lambda,num) {

  if( Math.abs(a-lambda) > near0 || Math.abs(b) > near0 ){
    eigen_vec[num] = new Vec2d(b, -mat_a + lambda);
  } else if( Math.abs(d-lambda) > near0 || Math.abs(c) > near0 ){
    eigen_vec[num] = new Vec2d(d-lambda, -c);
  } else {
    f_draw_all = 1;
  }
}

////////////////////////////////
// ベクトルクラス
////////////////////////////////
function Vec2d(x, y) {
  this.x = x;
  this.y = y;
}
/////////////////////////////////
// ベクトル描画
/////////////////////////////////
function draw_vector( x, y, col ){

  var px1, py1; // 先頭の位置
  var px2, py2; // 横（上側）
  var px3, py3; // 線の付け根
  var px4, py4; // 横（下側）
  var size = 15; // 頭の大きさ
  var modosi = 0.8; // sizeに対する戻しの割合
  var slim = 0.5; // 先頭部分の横幅、sizeに対する割合
  var result;
  var angle;

  // 数学的な平面で計算
  px1 = 0; py1 = 0;
  px2 = -size; py2 = 0.577*size*slim; 
  px3 = -size; py3 = 0;
  px4 = -size; py4 = -0.577*size*slim;

  // 回転変換
  if(x==0){
    if(y>=0){angle = Math.PI/2;}else{angle = Math.PI*3/2;}
  } else {
    angle = Math.atan(y/x);
    if(x<0){ angle += (Math.PI); }
  }

  result = rotation( px2, py2, angle );
  px2 = result[0];
  py2 = result[1];

  result = rotation( px3, py3, angle );
  px3 = result[0];
  py3 = result[1];

  result = rotation( px4, py4, angle );
  px4 = result[0];
  py4 = result[1];

  // 先頭部
  ctx.beginPath();
  ctx.fillStyle = col;
  ctx.lineWidth = 1;
  ctx.moveTo(px1+width/2+scale*x,height/2-py1-scale*y);
  ctx.lineTo(px2+width/2+scale*x,height/2-py2-scale*y);
  ctx.lineTo(px3+width/2+scale*x,height/2-py3-scale*y);
  ctx.lineTo(px4+width/2+scale*x,height/2-py4-scale*y);
  ctx.closePath();
  ctx.fill();

  // 直線部
  ctx.beginPath();
  ctx.lineWidth = 4;
  ctx.lineCap = "round";
  ctx.strokeStyle = col;
  ctx.moveTo(    width/2,  height/2 );
  ctx.lineTo(px3+width/2+scale*x,height/2-py3-scale*y);
  ctx.stroke();

}

///////////////////////////////
// 座標の回転
///////////////////////////////
function rotation( x, y, angle ){

  var nx,ny;
  var sin = Math.sin(angle);
  var cos = Math.cos(angle);

  nx = cos*x-sin*y;
  ny = sin*x+cos*y;

  return [nx,ny];
}

/////////////////////////////////
// 傾きy/xで座標原点を通る直線を描く
/////////////////////////////////
function draw_line( x, y ){

  // 画面外を指定しても正しく処理してくれるようなので、頼って簡単に済ませる。
  var angle;
  if(x==0){
    angle = Math.PI/2;
  } else {
    angle = Math.atan(y/x);
  }
  var nx = 500 * Math.cos( angle );
  var ny = 500 * Math.sin( angle );

  ctx.beginPath();
  ctx.lineWidth = 1;
  ctx.lineCap = "round";
  ctx.strokeStyle = "rgb(255,0,0)";
  ctx.moveTo(nx+width/2,-ny+height/2);
  ctx.lineTo(width/2,height/2);          // ズレを防ぐため原点を経由
  ctx.lineTo(-nx+width/2,ny+height/2);
  ctx.stroke();
}

/////////////////////////////////
// 固有ベクトルが全方向の場合の描画
/////////////////////////////////
function draw_all_direction(){

  ctx.beginPath();
  var gr = ctx.createRadialGradient(width/2, height/2, 0, width/2, height/2, width/2);
  gr.addColorStop(0.0,"rgb(255,180,180)");
  gr.addColorStop(1.0,"rgb(255,255,255)");
  ctx.fillStyle = gr;
  ctx.arc(width/2, height/2, width, 0, Math.PI*2, false);
  ctx.fill();
}

/////////////////////////////////
// 座標軸の描画
/////////////////////////////////
function draw_coordinates(){

  ctx.strokeStyle = "rgb(0,0,0)"; // 線は黒で

  // 縦線表示
  var i;
  ctx.lineWidth = 0.2;
  for( i=-3; i<4; i++ ){
    ctx.beginPath();
    ctx.moveTo(width/2+i*scale,0);
    ctx.lineTo(width/2+i*scale,height);
    ctx.stroke();
  }

  // 横線表示
  for( i=-3; i<4; i++ ){
    ctx.beginPath();
    ctx.moveTo(0,height/2+i*scale);
    ctx.lineTo(width,height/2+i*scale);
    ctx.stroke();
  }

  // x軸表示
  ctx.beginPath();
  ctx.lineWidth = 1.0;
  ctx.moveTo(0,height/2);
  ctx.lineTo(width,height/2);
  ctx.stroke();

  // y軸表示
  ctx.beginPath();
  ctx.moveTo(width/2,0);
  ctx.lineTo(width/2,height);
  ctx.stroke();

  var str = "O";
  ctx.font = "14px sans-serif";
  ctx.fillStyle = "rgb(0, 0, 0)";
  ctx.fillText(str,width/2-15,height/2+16);

}

////////////////////////////////
// 平行四辺形の描画
////////////////////////////////
function draw_parallelograms(){

  // 一辺の長さ 1 の正方形
  ctx.beginPath();
  ctx.fillStyle = "rgba(200, 230, 230, 0.7)";
  ctx.moveTo( width/2,         height/2 );
  ctx.lineTo( width/2 + scale, height/2 );
  ctx.lineTo( width/2 + scale, height/2 - scale );
  ctx.lineTo( width/2,         height/2 - scale );
  ctx.fill();

  // 変換後の正方形
  ctx.beginPath();
  ctx.fillStyle = "rgba(200, 200, 255, 0.65)";
  ctx.moveTo( width/2,                 height/2 );
  ctx.lineTo( width/2 + mat_a * scale, height/2 - mat_c * scale);
  ctx.lineTo( width/2 + (mat_a+mat_b)*scale, height/2 - (mat_c+mat_d)*scale );
  ctx.lineTo( width/2 + mat_b * scale, height/2 - mat_d * scale );
  ctx.fill();

}

////////////////////////////////
// サンプルベクトルの表示
////////////////////////////////
function draw_samples() {

  var x,y,sx,sy;

  sx = radius_s * Math.cos( angle_s/360*2*Math.PI );
  sy = radius_s * Math.sin( angle_s/360*2*Math.PI );
  draw_vector( sx,sy,"rgb(0,0,0)" );

  x = mat_a * sx + mat_b * sy;
  y = mat_c * sx + mat_d * sy;

  draw_vector( x,y,"rgb(255,0,255)" );
}

////////////////////////////////
// 数値情報の表示
////////////////////////////////
function disp_numbers() {

  var str;
  var tm;

  // 変換行列の表示
  ctx.fillStyle = "rgb(0, 0, 0)";

  str = "A = ";
  ctx.font = "15px sans-serif";
  tm = ctx.measureText(str);
  ctx.fillText(str,width-118-tm.width, 45);

  ctx.drawImage(img_kakko, width-117, 11, 110, 56);  // 元画像は 83*51

  ctx.fillStyle = "rgb(0, 0, 0)";

  str = ""+mat_a.toFixed(2);
  ctx.font = "15px sans-serif";
  tm = ctx.measureText(str);
  ctx.fillText(str,width-65-tm.width, 30);

  str = ""+mat_b.toFixed(2);
  ctx.font = "15px sans-serif";
  tm = ctx.measureText(str);
  ctx.fillText(str,width-20-tm.width, 30);

  str = ""+mat_c.toFixed(2);
  ctx.font = "15px sans-serif";
  tm = ctx.measureText(str);
  ctx.fillText(str,width-65-tm.width, 57);

  str = ""+mat_d.toFixed(2);
  ctx.font = "15px sans-serif";
  tm = ctx.measureText(str);
  ctx.fillText(str,width-20-tm.width, 57);

  // 固有値の表示
  if( vec_num == 0 ){
    str = "λ = ";
    ctx.font = "15px sans-serif";
    tm = ctx.measureText(str);
    ctx.fillText(str,width-118-tm.width, 90);

    str = "" + lambda1.toFixed(2) + " ± " + imaginary.toFixed(2) + " i";
    ctx.font = "15px sans-serif";
    ctx.fillText(str,width-117, 90);

  } else if( vec_num == 1 ){
    str = "λ = ";
    ctx.font = "15px sans-serif";
    tm = ctx.measureText(str);
    ctx.fillText(str,width-118-tm.width, 90);

    str = "" + lambda1.toFixed(2);
    ctx.font = "15px sans-serif";
    ctx.fillText(str,width-117, 90);

    // デバグ用
    str = "A-λE = ";
    ctx.font = "15px sans-serif";
    tm = ctx.measureText(str);
    ctx.fillText(str,width-118-tm.width, 45+90);

    ctx.drawImage(img_kakko, width-117, 11+90, 110, 56);

    //str = "(        )";
    //ctx.font = "60px sans-serif";
    //ctx.fillText(str,width-93, 57+90, 90);

    ctx.fillStyle = "rgb(0, 0, 0)";

    str = ""+(mat_a-lambda1).toFixed(2);
    ctx.font = "15px sans-serif";
    tm = ctx.measureText(str);
    ctx.fillText(str,width-65-tm.width, 30+90);

    str = ""+mat_b.toFixed(2);
    ctx.font = "15px sans-serif";
    tm = ctx.measureText(str);
    ctx.fillText(str,width-20-tm.width, 30+90);

    str = ""+mat_c.toFixed(2);
    ctx.font = "15px sans-serif";
    tm = ctx.measureText(str);
    ctx.fillText(str,width-65-tm.width, 57+90);

    str = ""+(mat_d-lambda1).toFixed(2);
    ctx.font = "15px sans-serif";
    tm = ctx.measureText(str);
    ctx.fillText(str,width-20-tm.width, 57+90);

  } else if( vec_num == 2 ){
    str = "λ = ";
    ctx.font = "15px sans-serif";
    tm = ctx.measureText(str);
    ctx.fillText(str,width-118-tm.width, 90);

    str = "" + lambda1.toFixed(2) + "  , " + lambda2.toFixed(2);
    ctx.font = "15px sans-serif";
    ctx.fillText(str,width-117, 90);

    // デバグ用
    str = "A-λE = ";
    ctx.font = "15px sans-serif";
    tm = ctx.measureText(str);
    ctx.fillText(str,width-118-tm.width, 45+90);

    ctx.drawImage(img_kakko, width-117, 11+90, 110, 56);

    ctx.fillStyle = "rgb(0, 0, 0)";

    str = ""+(mat_a-lambda1).toFixed(2);
    ctx.font = "15px sans-serif";
    tm = ctx.measureText(str);
    ctx.fillText(str,width-65-tm.width, 30+90);

    str = ""+mat_b.toFixed(2);
    ctx.font = "15px sans-serif";
    tm = ctx.measureText(str);
    ctx.fillText(str,width-20-tm.width, 30+90);

    str = ""+mat_c.toFixed(2);
    ctx.font = "15px sans-serif";
    tm = ctx.measureText(str);
    ctx.fillText(str,width-65-tm.width, 57+90);

    str = ""+(mat_d-lambda1).toFixed(2);
    ctx.font = "15px sans-serif";
    tm = ctx.measureText(str);
    ctx.fillText(str,width-20-tm.width, 57+90);

    /////

    str = "A-λE = ";
    ctx.font = "15px sans-serif";
    tm = ctx.measureText(str);
    ctx.fillText(str,width-118-tm.width, 45+160);

    ctx.drawImage(img_kakko, width-117, 11+160, 110, 56);

    ctx.fillStyle = "rgb(0, 0, 0)";

    str = ""+(mat_a-lambda2).toFixed(2);
    ctx.font = "15px sans-serif";
    tm = ctx.measureText(str);
    ctx.fillText(str,width-65-tm.width, 30+160);

    str = ""+mat_b.toFixed(2);
    ctx.font = "15px sans-serif";
    tm = ctx.measureText(str);
    ctx.fillText(str,width-20-tm.width, 30+160);

    str = ""+mat_c.toFixed(2);
    ctx.font = "15px sans-serif";
    tm = ctx.measureText(str);
    ctx.fillText(str,width-65-tm.width, 57+160);

    str = ""+(mat_d-lambda2).toFixed(2);
    ctx.font = "15px sans-serif";
    tm = ctx.measureText(str);
    ctx.fillText(str,width-20-tm.width, 57+160);

  }
}

////////////////////////////////
// タイマーイベント関数
////////////////////////////////
function timerfunc() {
  var moving = 0;

  if( key_angle_up == 1 ){
    moving = 1;
    if(key_sample){
      angle_s += dif_angle;
      if( angle_s >= 360 ) angle_s -= 360;
    } else if(key_space){
      angle_y += dif_angle;
      if( angle_y >= 360 ) angle_y -= 360;
    } else {
      angle_x += dif_angle;
      if( angle_x >= 360 ) angle_x -= 360;
    }
  }
  if( key_angle_down == 1 ){
    moving = 1;
    if(key_sample){
      angle_s -= dif_angle;
      if( angle_s < 0 ) angle_s += 360;
    } else if(key_space){
      angle_y -= dif_angle;
      if( angle_y < 0 ) angle_y += 360;
    } else {
      angle_x -= dif_angle;
      if( angle_x < 0 ) angle_x += 360;
    }
  }
  if( key_radius_up == 1 ){
    moving = 1;
    if(key_sample){
      radius_s += dif_radius;
      if( radius_s >= 100 ) radius_s = 100;
    } else if(key_space){
      radius_y += dif_radius;
      if( radius_y >= 100 ) radius_y = 100;
    } else {
      radius_x += dif_radius;
      if( radius_x >= 100 ) radius_x = 100;
    }
  }
  if( key_radius_down == 1 ){
    moving = 1;
    if(key_sample){
      radius_s -= dif_radius;
      if( radius_s < 0.1 ) radius_s = 0.1;
    } else if(key_space){
      radius_y -= dif_radius;
      if( radius_y < 0.1 ) radius_y = 0.1;
    } else {
      radius_x -= dif_radius;
      if( radius_x < 0.1 ) radius_x = 0.1;
    }
  }
  if( key_angle_up_y == 1 ){
    moving = 1;
    angle_y += dif_angle;
    if( angle_y >= 360 ) angle_y -= 360;
  }
  if( key_angle_down_y == 1 ){
    moving = 1;
    angle_y -= dif_angle;
    if( angle_y < 0 ) angle_y += 360;
  }
  if( key_radius_up_y == 1 ){
    moving = 1;
    radius_y += dif_radius;
    if( radius_y >= 100 ) radius_y = 100;
  }
  if( key_radius_down_y == 1 ){
    moving = 1;
    radius_y -= dif_radius;
    if( radius_y < 0.1 ) radius_y = 0.1;
  }
  if( key_reset == 1 ){
    moving = 1;
    key_reset = 0;
    init_vars();
  }
  if( key_sample_mode ){
    moving = 1;
    key_sample_mode = 0;
    sample_mode = !sample_mode;
    enable_sample_buttons(sample_mode);
  }

  // ボタン
  if( btn_reset ){
    moving = 1;
    btn_reset = 0;
    init_vars();
    enable_sample_buttons(false);

  }

  if( btn_x ){
    moving = 1;
    btn_x = 0;
    sample_mode = !sample_mode;
    enable_sample_buttons(sample_mode);
  }

  // 十字ボタン

  // x ベクトル
  if( btn_x_angle_up ){
    moving = 1;
    angle_x += dif_angle;
    if( angle_x >= 360 ) angle_x -= 360;
  }

  if( btn_x_angle_down ){
    moving = 1;
    angle_x -= dif_angle;
    if( angle_x < 0 ) angle_x += 360;
  }

  if( btn_x_radius_up ){
    moving = 1;
    radius_x += dif_radius;
    if( radius_x >= 100 ) radius_x = 100;
  }

  if( btn_x_radius_down ){
    moving = 1;
    radius_x -= dif_radius;
    if( radius_x < 0.1 ) radius_x = 0.1;
  }

  // y ベクトル
  if( btn_y_angle_up ){
    moving = 1;
    angle_y += dif_angle;
    if( angle_y >= 360 ) angle_y -= 360;
  }

  if( btn_y_angle_down ){
    moving = 1;
    angle_y -= dif_angle;
    if( angle_y < 0 ) angle_y += 360;
  }

  if( btn_y_radius_up ){
    moving = 1;
    radius_y += dif_radius;
    if( radius_y >= 100 ) radius_y = 100;
  }

  if( btn_y_radius_down ){
    moving = 1;
    radius_y -= dif_radius;
    if( radius_y < 0.1 ) radius_y = 0.1;
  }

  // s ベクトル
  if( btn_s_angle_up ){
    moving = 1;
    angle_s += dif_angle;
    if( angle_s >= 360 ) angle_s -= 360;
  }

  if( btn_s_angle_down ){
    moving = 1;
    angle_s -= dif_angle;
    if( angle_s < 0 ) angle_s += 360;
  }

  if( btn_s_radius_up ){
    moving = 1;
    radius_s += dif_radius;
    if( radius_s >= 100 ) radius_s = 100;
  }

  if( btn_s_radius_down ){
    moving = 1;
    radius_s -= dif_radius;
    if( radius_s < 0.1 ) radius_s = 0.1;
  }


  if( moving == 1){
    draw();
    show_all_buttons();
  }
}


/////////////////////////////////////
// キーボードイベント関数
/////////////////////////////////////
function keyupfunc(event) {
  var code = event.keyCode;
  switch (code) {
  case 32:    // スペースキー
    key_space = 0;
    event.preventDefault();
    break;
  case 37:    // ←キー
    key_angle_up = 0;
    event.preventDefault();
    break;
  case 39:    // →キー
    key_angle_down = 0;
    event.preventDefault();
    break;
  case 38:    // ↑キー
    key_radius_up = 0;
    event.preventDefault();
    break;
  case 40:    // ↓キー
    key_radius_down = 0;
    event.preventDefault();
    break;
  case 65:    // A
    key_angle_up_y = 0;
    event.preventDefault();
    break;
  case 83:    // S
    key_radius_down_y = 0;
    event.preventDefault();
    break;
  case 68:    // D
    key_angle_down_y = 0;
    event.preventDefault();
    break;
  case 87:    // W
    key_radius_up_y = 0;
    event.preventDefault();
    break;
  case 90:    // Z
    key_sample = 0;
    event.preventDefault();
    break;
   
  }
}

function keydownfunc(event) {
  var code = event.keyCode;
  if (window.opera) {
    // フォーカスオブジェクトの設定
    document.focusform.focustext.focus();
  }
  switch (code) {
  case 32:    // スペースキー
    key_space = 1;
    event.preventDefault();
    break;
  case 37:    // ←キー
    key_angle_up = 1;
    event.preventDefault();
    break;
  case 39:    // →キー
    key_angle_down = 1;
    event.preventDefault();
    break;
  case 38:    // ↑キー
    key_radius_up = 1;
    event.preventDefault();
    break;
  case 40:    // ↓キー
    key_radius_down = 1;
    event.preventDefault();
    break;
  case 65:    // A
    key_angle_up_y = 1;
    event.preventDefault();
    break;
  case 83:    // S
    key_radius_down_y = 1;
    event.preventDefault();
    break;
  case 68:    // D
    key_angle_down_y = 1;
    event.preventDefault();
    break;
  case 87:    // W
    key_radius_up_y = 1;
    event.preventDefault();
    break;
  case 82:    // R
    key_reset = 1;
    event.preventDefault();
    break;
  case 88:    // X
    key_sample_mode = 1;
    event.preventDefault();
    break;
  case 90:    // Z
    key_sample = 1;
    event.preventDefault();
    break;
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

  // どのボタンの上で押されたかを調べる
  var i;
  for( i=0; i<15; i++ ){
    if( btn[i].over ){
      change_btn_flags(i,1);
      press_no = i;
      break;
    }
  }

}

////////////////////////////////
// マウスボタンが離されたときの処理
////////////////////////////////
function onMouseUp(event) {

  if( press_no >= 0 ){
    change_btn_flags(press_no,0);
    press_no = -1;
  }
}

////////////////////////////////
// 指が画面に触れたときの処理
////////////////////////////////
function onTouchStart(event) {

  touch_check1(event);    // 十字ボタン系
  touch_check2(event);    // 押しボタン系
  event.preventDefault(); // 同時発生するマウスイベントを消す
}

////////////////////////////////
// 指が画面上で動いたときの処理
////////////////////////////////
function onTouchMove(event) {

  touch_check1(event);
}

////////////////////////////////
// 指が画面から離されたときの処理
////////////////////////////////
function onTouchEnd(event) {

  touch_check1(event);
}

////////////////////////////////
// タッチイベントが発生したときの処理（十字ボタン）
////////////////////////////////
function touch_check1( event ){

  var num;
  var i;

  // 一旦フラグを全部下げる
  for(i=0;i<12;i++){
    change_btn_flags(i,0);
  }

  num = event.touches.length;
  if( num == 0 ){
    return;
  }

  var rect = event.target.getBoundingClientRect();
  var x;
  var y;

  for( i=0; i<num; i++ ){
    x = event.touches[i].pageX - rect.left - window.pageXOffset;
    y = event.touches[i].pageY - rect.top - window.pageYOffset;;
    touch_check_sub1(x,y);
  }
}


////////////////////////////////
// 指が乗っている場所全部チェック
////////////////////////////////
function touch_check_sub1( fingerX, fingerY){

  var x;
  var y;
  var w;
  var h;

  for(i=0;i<8;i++){
    x = btn[i].x;
    y = btn[i].y;
    w = btn[i].w;
    h = btn[i].h;
    if( fingerX>x && fingerX<x+w && fingerY>y && fingerY<y+h ){
      change_btn_flags(i,1);
    }
  }

  if( sample_mode ){
    for(i=8;i<12;i++){
      x = btn[i].x;
      y = btn[i].y;
      w = btn[i].w;
      h = btn[i].h;
      if( fingerX>x && fingerX<x+w && fingerY>y && fingerY<y+h ){
        change_btn_flags(i,1);
      }
    }
  }
}

////////////////////////////////
// タッチイベントが発生したときの処理（押しボタン）
////////////////////////////////
function touch_check2( event ){

  var num;
  var i;

  num = event.touches.length;
  if( num == 0 ) return;    // 指が全て離れた

  var rect = event.target.getBoundingClientRect();
  var x;
  var y;

  for( i=0; i<num; i++ ){
    x = event.touches[i].pageX - rect.left - window.pageXOffset;
    y = event.touches[i].pageY - rect.top - window.pageYOffset;;
    touch_check_sub2(x,y);
  }
}

////////////////////////////////
// 指が乗っている場所全部チェック
////////////////////////////////
function touch_check_sub2( fingerX, fingerY){

  var x;
  var y;
  var w;
  var h;

  for(i=12;i<14;i++){
    x = btn[i].x;
    y = btn[i].y;
    w = btn[i].w;
    h = btn[i].h;
    if( fingerX>x && fingerX<x+w && fingerY>y && fingerY<y+h ){
      change_btn_flags(i,1);
    }
  }
}

// ========================= ボタン制御 ============================

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

/////////////////////////////////
// ボタン登録
/////////////////////////////////
function regist_buttons() {

  var cross_y = 640;
  var top = 480;

  // 緑ベクトル操作（右側配置）
  regist_cross_button( 0, 320+180, cross_y );

  // 青ベクトル操作（左側配置）
  regist_cross_button( 4, 320-180, cross_y );

  // 黒ベクトル操作（中央配置）
  regist_cross_button( 8, 320, cross_y );

  // [R] リセットボタン
  btn[12] = new button( 20, top + 20, 80, 40, 5 );

  // [X] ボタン
  btn[13] = new button( 320-20, 500, 40, 40, 5 );

}

/////////////////////////////////
// 十字キーワンセット登録
/////////////////////////////////
function regist_cross_button( n, x, y ) {

  // x, y は中心位置とする

  var width_ud = 40;
  var height_ud = 50;
  var gap_ud = 25;  // 中心から一番近い辺までの距離

  var width_lr = 50;
  var height_lr = 40;
  var gap_lr = 25;

  // 上キー
  btn[n] = new button( x-width_ud/2, y-gap_ud-height_ud, width_ud, height_ud, 5 );

  // 下キー
  btn[n+1] = new button( x-width_ud/2, y+gap_ud, width_ud, height_ud, 5 );

  // 左キー
  btn[n+2] = new button( x-gap_lr-width_lr, y-height_lr/2, width_lr, height_lr, 5 );

  // 右キー
  btn[n+3] = new button( x+gap_lr, y-height_lr/2, width_lr, height_lr, 5 );


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
      change_btn_flags(i,0);
      btn[i].over = false;
    } else if( btn[i].over ){
      if( !is_over_button(i) ){
        show_button(i,0);
        change_btn_flags(i,0);
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
// ボタンのフラグ書き換え
////////////////////////////////
function change_btn_flags (i,f) {

  switch(i){
    case 0:
      btn_x_radius_up = f;
      break;
    case 1:
      btn_x_radius_down = f;
      break;
    case 2:
      btn_x_angle_up = f;
      break;
    case 3:
      btn_x_angle_down = f;
      break;
    case 4:
      btn_y_radius_up = f;
      break;
    case 5:
      btn_y_radius_down = f;
      break;
    case 6:
      btn_y_angle_up = f;
      break;
    case 7:
      btn_y_angle_down = f;
      break;
    case 8:
      btn_s_radius_up = f;
      break;
    case 9:
      btn_s_radius_down = f;
      break;
    case 10:
      btn_s_angle_up = f;
      break;
    case 11:
      btn_s_angle_down = f;
      break;
    case 12:
      btn_reset = f;
      break;
    case 13:
      btn_x = f;
      break;
  }
}

////////////////////////////////
// 全ボタン作成（全てのボタンを書き直す）
////////////////////////////////
function show_all_buttons() {

  var i;

  ctx.fillStyle = "rgb(20, 20, 20)";
  ctx.fillRect(0, height, width, 250);

  for( i=0; i<14; i++ ){
    if( !btn[i].enable ){
      show_button(i,-1);
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

  if( n<4 ){ // 緑
    if( f==1 ){
      ctx.fillStyle = "rgb(150, 230, 150)"; // 光る
    } else if( f==0 ){
      ctx.fillStyle = "rgb(60, 180, 60)";
    }
  } else if( n<8 ){ // 青
    if( f==1 ){
      ctx.fillStyle = "rgb(160, 160, 250)"; // 光る
    } else {
      ctx.fillStyle = "rgb(60, 60, 180)";
    }
  } else if( n<12 ){ // 黒
    if( f==1 ){
      ctx.fillStyle = "rgb(180, 180, 180)"; // 光る
    } else if( f==0 ){
      ctx.fillStyle = "rgb(80, 80, 80)";
    } else {
      ctx.fillStyle = "rgb(20, 20, 20)"; // 無効
    }
  } else if( n==12 ){ // R リセット
    if( f==1 ){
      ctx.fillStyle = "rgb(250, 160, 160)"; // 光る
    } else {
      ctx.fillStyle = "rgb(185, 60, 60)";
    }
  } else if( n==13 ){ // X ボタン
    if( f==1 ){
      ctx.fillStyle = "rgb(180, 180, 200)"; // 光る
    } else {
      ctx.fillStyle = "rgb(80, 80, 80)";
    }
  }
  fillRoundRect(x, y, w, h, c-3 );

  if( n<12 ) return;

  // ボタンの上に文字を書く
  var str;

  ctx.font = "bold 18px sans-serif";

  if( f==-1 ){
    ctx.fillStyle = "rgb(100, 100, 100)"; // 無効
  } else {
    ctx.fillStyle = "rgb(255, 255, 255)";
  }

  if( n==12 ){ // リセットボタン
      str = "RESET";
      ctx.fillText(str, x + 9, y + 26);
  } else if( n==13 ){ // Xボタン
      str = "X";
      ctx.fillText(str, x + 12, y + 26);
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

////////////////////////////////
// サンプルベクトル用の十字ボタンの有効・無効
////////////////////////////////
function enable_sample_buttons(f){

  enable_button(8,f);
  enable_button(9,f);
  enable_button(10,f);
  enable_button(11,f);
}


