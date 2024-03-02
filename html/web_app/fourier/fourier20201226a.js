///////////////////////////////////////////////////////////////
// フーリエ級数実演
// Copyright (c) 2012 K.Hiroe

// 注意：素人が見様見真似で作ったものなので参考にすべきでない。


//////////////////////////
// 大域変数
//////////////////////////
var width = 640;
var height = 530;

var side_margin = 20;
var graph_width = 600;
var graph_height = 380;
var pnum = 600;  // 形状データの点の個数(graph_widthと同じになる)
var RATE = 0.15;
var RATE2 = 0.05;
var TURN = 310;


var ctx;           // canvasコンテキスト
var timerID;
var PI;
var counter;
var scene;

// 1: 初期状態（マウスをここへ持ってきてスタートという案内）
// 2: 描画中
// 3: 描画自動終了中
// 4: 計算中という表示
// 5: 平均値を表示（「全体の平均はこれくらいです。ここから始めましょう」）
// 6: 次の波を表示（「次にこの波を加えます」）
// 7: 合成中（合成が終わったらそのまま待機）→ 「次へ」ボタンで 6 に移る。

var order;

var mouseX, mouseY;
var penX, penY;
var f_goal_message;

var dataX = [];
var dataY = [];
var data_idx;

var ave;
var factor_cos = [];
var factor_sin = [];

var form = [];
var base = [];
var add = [];


var f_over_reset;
var f_over_next;
var f_over_prev;
var f_over_ff;
var f_over_rew;

var f_enable_next;
var f_enable_prev;
var f_enable_ff;
var f_enable_rew;

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
  first_message();
  show_buttons();
  disp_message_start();

  // マウスイベント関数の設定
  canvas.addEventListener('mousemove', updateMousePos, false);
  canvas.addEventListener('mouseout', resetMousePos, false);
  canvas.addEventListener('click', onClick, false);

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

  scene = 1;

  data_idx = 1;
  dataX[0] = side_margin;
  dataY[0] = graph_height/2;

  mouseX = 100;
  mouseY = 100;
  penX = side_margin;
  penY = graph_height/2;

  f_goal_message = 0;

  PI = Math.PI;

  f_over_reset = false;
  f_over_next = false;
  f_over_prev = false;
  f_over_ff = false;
  f_over_rew = false;

  f_enable_next = false;
  f_enable_prev = false;
  f_enable_ff = false;
  f_enable_rew = false;

}

/////////////////////////////////
// 枠の準備
/////////////////////////////////
function draw_frames() {

  ctx.fillStyle="rgb(255,255,255)";
  ctx.fillRect(0,0,width,height);

  ctx.strokeStyle = "rgb(0,0,0)";
  ctx.lineWidth = 1;
  ctx.lineCap = "square";
  ctx.beginPath();

  // 左縦線
  ctx.moveTo( side_margin, 0 );
  ctx.lineTo( side_margin, graph_height );
  // 右縦線
  ctx.moveTo( width-side_margin, 0 );
  ctx.lineTo( width-side_margin, graph_height );
  // 中心横線
  ctx.moveTo( side_margin, graph_height/2 );
  ctx.lineTo( width-side_margin, graph_height/2 );
  // 中心目盛
  ctx.moveTo( width/2, graph_height/2-6 );
  ctx.lineTo( width/2, graph_height/2+6 );
  // 左小目盛
  ctx.moveTo( side_margin+graph_width/4, graph_height/2-3 );
  ctx.lineTo( side_margin+graph_width/4, graph_height/2+3 );
  // 右小目盛
  ctx.moveTo( side_margin+graph_width/4*3, graph_height/2-3 );
  ctx.lineTo( side_margin+graph_width/4*3, graph_height/2+3 );
  ctx.stroke();

  // メッセージ枠
  ctx.strokeStyle = "rgb(0,55,255)";
  ctx.lineCap = "round";
  ctx.strokeRect( 3, graph_height+3, width-6, 150-6);

}


/////////////////////////////////
// 開始時の吹出し
/////////////////////////////////
function first_message() {

  // 開始点の円
  ctx.strokeStyle = "rgb(200,0,0)";
  ctx.lineWidth = 2;
  ctx.beginPath();
  ctx.arc( side_margin, graph_height/2, 6, 0, Math.PI*2, false );
  ctx.stroke();

  // 開始メッセージ吹出し
  ctx.fillStyle = "rgb(190,190,240)";
  ctx.beginPath();
  ctx.moveTo( 70,            graph_height/2+40 );
  ctx.lineTo( side_margin+5, graph_height/2+5 );
  ctx.lineTo( 50,            graph_height/2+60 );
  ctx.closePath();
  ctx.fill();

  fillRoundRect(50, graph_height/2+40, 300, 110, 20 );

  var str;
  ctx.fillStyle = "rgb(0, 0, 0)";
  ctx.font = "18px sans-serif";
  str = "始めるために";
  ctx.fillText(str, 80, graph_height/2+70);
  str = "指 または マウスカーソルを";
  ctx.fillText(str, 90, graph_height/2+100);
  str = "この円の辺りに置いて下さい";
  ctx.fillText(str, 80, graph_height/2+130);


}

/////////////////////////////////
// ゴールを促す吹出し
/////////////////////////////////
function goal_message() {

  // 目標地点メッセージ吹出し

  ctx.fillStyle = "rgb(190,190,240)";
  ctx.beginPath();
  ctx.moveTo( 575,            140 );
  ctx.lineTo( width-side_margin-3, graph_height/2-3 );
  ctx.lineTo( 545,            150 );
  ctx.closePath();
  ctx.fill();

  fillRoundRect(365, 50, 220, 110, 20 );

  var str;
  ctx.fillStyle = "rgb(0, 0, 0)";
  ctx.font = "18px sans-serif";
  str = "この辺りを目指して";
  ctx.fillText(str, 380, 80);
  str = "自由に線を";
  ctx.fillText(str, 430, 110);
  str = "引いてみて下さい";
  ctx.fillText(str, 410, 140);

}

/////////////////////////////////
// ゴールを促す吹出しを消す
/////////////////////////////////
function delete_goal_message() {

  ctx.fillStyle = "rgb(255,255,255)";
  ctx.fillRect(355,40,240,130);
  ctx.fillRect(570,135,47,53);
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
// ユーザーのマウスが定位置に来るのを待つ
////////////////////////////////
function wait_user_mouse() {

  if( Math.abs(mouseX-side_margin)<20 && Math.abs(mouseY-graph_height/2)<20 ){
    scene = 2;
    draw_frames();
    show_buttons();
    disp_message_start();
    goal_message();
    f_goal_message = 1;
  }
}

////////////////////////////////
// ユーザーの描く曲線情報を取得
////////////////////////////////
function get_user_line() {

  if( mouseX<0 ) return;
  if( mouseY<0 || mouseY > graph_height-5 ) return;

  // 終了判定
  if( mouseX > width-side_margin-15 ){
    scene = 3;
  }

  if( f_goal_message == 1 ){
    if( mouseX > width/2 ){
      f_goal_message = 0;
      delete_goal_message();
    }
  }

  var penVX = (mouseX - penX)*RATE;
  var penVY = (mouseY - penY)*RATE;

  preX = penX;
  preY = penY;

  if( penVX>0 ){
    penX += penVX;
    penY += penVY;
  }

  if( Math.floor(dataX[data_idx]) != Math.floor(penX) ){
    dataX[data_idx] = Math.floor(penX);
    dataY[data_idx] = Math.floor(penY);
    data_idx ++;
  }

  ctx.fillStyle = "rgb(0,0,255)";
  ctx.lineWidth = 3;
  ctx.lineCap = "round";
  ctx.strokeStyle = "rgb(255,0,0)";
  ctx.beginPath();
  ctx.moveTo(preX,preY);
  ctx.lineTo(penX,penY);
  ctx.stroke();

}

////////////////////////////////
// 曲線の自動終了
////////////////////////////////
function auto_user_line() {

  // 目標地点を固定する
  var X = width-side_margin;
  var Y = graph_height/2;

  var penVX = (X - penX) * RATE2;
  var penVY = (Y - penY) * RATE2;

  preX = penX;
  preY = penY;

  if( penVX>0 ){
    penX += penVX;
    penY += penVY;
  }

  if( Math.floor(dataX[data_idx]) != Math.floor(penX) ){
    dataX[data_idx] = Math.floor(penX);
    dataY[data_idx] = Math.floor(penY);
    data_idx ++;
  }

  ctx.fillStyle = "rgb(0,0,255)";
  ctx.lineWidth = 3;
  ctx.lineCap = "round";
  ctx.strokeStyle = "rgb(255,0,0)";
  ctx.beginPath();
  ctx.moveTo(preX,preY);
  ctx.lineTo(penX,penY);
  ctx.stroke();

  if( penX >= X-1 ){
    ctx.beginPath();
    ctx.moveTo(penX,penY);
    ctx.lineTo(X,Y);
    ctx.stroke();

    scene = 4;
    calculation();
    counter = 0;
  }

}

////////////////////////////////
// タイマーイベント関数
////////////////////////////////
function timerfunc() {

  switch(scene){
    case 1:
      wait_user_mouse();
      break;

    case 2:
      get_user_line();
      break;

    case 3:
      auto_user_line();
      break;

    case 4:
      wait_for_calculation();
      break;

    case 5:
      break;

    case 6:
      break;

    case 7:
      animation();
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
function onClick(event) {

  if( f_over_reset ){
    init_vars();
    draw_frames();
    first_message();
    show_buttons();
    disp_message_start();
    return;
  }

  if( f_over_next ){

    switch( scene ){

      case 5:
        order = 1;
        goto_scene6();
        break;

      case 6:
        scene = 7;
        counter = 0;
        enable_prev(true);
        if( order >= 300 ){
          enable_next(false);
        }
        break;

      case 7:
        order ++;
        if( order > 300 ) order = 300;
        goto_scene6();
        break;
    }
    return;
  }

  if( f_over_prev ){

    if( scene <= 5 ) return;

    if( scene == 6 ){
      order --;
      if( order < 1 ) order = 1;
    }
    goto_scene6();
    return;
  }

  if( f_over_ff ){
    if( scene <= 4 ) return;

    order += 5;
    if( order > 300 ) order = 300;
    goto_scene6();
    return;
  }

  if( f_over_rew ){
    if( scene <= 5 ) return;

    order -= 5;
    if( order < 1 ) order = 1;
    goto_scene6();
    return;
  }
}


////////////////////////////////
// 指が画面に触れたときの処理
////////////////////////////////
function onTouchStart(event) {

  var rect = event.target.getBoundingClientRect();

  mouseX = event.changedTouches[0].pageX - rect.left - window.pageXOffset;
  mouseY = event.changedTouches[0].pageY - rect.top - window.pageYOffset;;

  check_buttons();
  onClick(event);

  event.preventDefault(); // 同時発生するマウスイベントを消す

}

////////////////////////////////
// 指が画面上で動いたときの処理
////////////////////////////////
function onTouchMove(event) {

  var rect = event.target.getBoundingClientRect();

  mouseX = event.changedTouches[0].pageX - rect.left - window.pageXOffset;
  mouseY = event.changedTouches[0].pageY - rect.top - window.pageYOffset;;

  check_buttons();

}

////////////////////////////////
// 指が画面から離されたときの処理
////////////////////////////////
function onTouchEnd(event) {

  mouseX = -100;
  mouseY = -100;

  check_buttons();

}



////////////////////////////////
// シーン６の画面を準備
////////////////////////////////
function goto_scene6() {

  scene = 6;

  order_button_check();

  prepare_add_data(order);
  prepare_base_data(order-1);
  draw_all(0);
  disp_message_term(order);
  disp_message_add();
}

////////////////////////////////
// 計算終了待ち
////////////////////////////////
function wait_for_calculation() {

  counter ++;

  if( counter>40 ){
    delete_message();
    disp_message_average();
    prepare_base_data(0);
    show_base_line();
    enable_next(true);
    enable_ff(true);
    order = 0;
    scene = 5;
  }
}

////////////////////////////////
// スペクトル分解
////////////////////////////////
function calculation() {

  delete_message();
  disp_message_cal();

  var i,j;

  // まずは波形の座標データを補間して、数学的なデータを作る。

  var form_idx = 0;
  var dx,dy;

  form[0] = 0;
  for( i=1; i<data_idx-1; i++ ){
    dx = (dataX[i] - side_margin) - form_idx;
    dy = dataY[i] - dataY[i-1];
    for( var j=1; j<=dx; j++ ){
      form_idx ++;
      form[form_idx] = -(j * dy/dx + dataY[i-1]) + graph_height/2;
    }
  }


  // 平均を計算
  ave = 0;
  for( i=0; i<pnum-1; i++ ){
    ave += form[i];
  }
  ave /= pnum;

  // 係数データをご破算
  for( i=0; i<TURN/2; i++ ){
    factor_cos[i] = 0;
    factor_sin[i] = 0;
  }

  // フーリエ係数を計算する。
  for( i=0; i<pnum-1; i++ ){
    for( j=1; j<TURN/2; j++ ){
      factor_sin[j]  += form[i] * Math.sin( j*2*PI*i/pnum );
      factor_cos[j]  += form[i] * Math.cos( j*2*PI*i/pnum );
    }
  }

  for( i=1; i<TURN/2; i++ ){
    factor_sin[i] /= (pnum/2);
    factor_cos[i] /= (pnum/2);
  }

}

////////////////////////////////
// 基本線の表示
////////////////////////////////
function show_base_line() {

  var i;

  ctx.beginPath();
  ctx.lineWidth = 2;
  ctx.lineCap = "round";
  ctx.strokeStyle = "rgb(0,0,255)";

  ctx.moveTo( side_margin, -base[0] + graph_height/2 );

  for( i=1; i<pnum-1; i++ ){
    ctx.lineTo( side_margin + i, -base[i] + graph_height/2 );
  }

  ctx.stroke();
}


////////////////////////////////
// n 次までの加算波形データを作成
////////////////////////////////
function prepare_base_data (n) {

  var i;
  var j;

  for( i=0; i<pnum; i++ ){
    base[i] = ave;
  }

  if( n==0 ) return;

  var half = Math.floor(n/2);

  for( i=0; i<pnum-1; i++ ){
    for( j=1; j<=half; j++ ){
      base[i] += factor_sin[j] * Math.sin( j*2*PI*i/pnum );
      base[i] += factor_cos[j] * Math.cos( j*2*PI*i/pnum );
    }
  }

  if( n % 2 == 1 ){
    for( i=0; i<pnum-1; i++ ){
      base[i] += factor_sin[half+1] * Math.sin( (half+1)*2*PI*i/pnum );
    }
  }
}


////////////////////////////////
// n 次項の加算データを作成
////////////////////////////////
function prepare_add_data (n) {

  var i;
  var k;

  if( n % 2 == 1 ){
    k = (n+1)/2;
    for( i=0; i<pnum; i++ ){
      add[i] = factor_sin[k] * Math.sin( k*2*PI*i/pnum );
    }
  } else {
    k = n/2;
    for( i=0; i<pnum; i++ ){
      add[i] = factor_cos[k] * Math.cos( k*2*PI*i/pnum );
    }
  }
}



////////////////////////////////
// メッセージ以外全て書き直す
////////////////////////////////
function draw_all(n) {

  draw_frames();
  show_buttons();

  ctx.lineCap = "round";

  // ユーザーライン
  ctx.lineWidth = 3;
  ctx.beginPath();
  ctx.strokeStyle = "rgb(255,0,0)";

  ctx.moveTo( side_margin, -form[0] + graph_height/2 );

  for( i=1; i<pnum-1; i++ ){
    ctx.lineTo( side_margin + i, -form[i] + graph_height/2 );
  }

  ctx.stroke();


  // ベースライン
  ctx.lineWidth = 2;
  ctx.beginPath();
  ctx.strokeStyle = "rgb(0,0,255)";

  ctx.moveTo( side_margin, -base[0] - add[i]*n/100 + graph_height/2 );

  for( i=1; i<pnum-1; i++ ){
    ctx.lineTo( side_margin + i, -base[i] - add[i]*n/100 + graph_height/2 );
  }

  ctx.stroke();

  if( n == 100 ) return;

  // 加算ライン
  ctx.lineWidth = 2;
  ctx.beginPath();
  ctx.strokeStyle = "rgb(50,200,50)";

  ctx.moveTo( side_margin, -add[0]*(100-n)/100 + graph_height/2 );
  for( i=1; i<pnum-1; i++ ){
    ctx.lineTo( side_margin + i, -add[i]*(100-n)/100 + graph_height/2 );
  }

  ctx.stroke();

}


////////////////////////////////
// アニメーション
////////////////////////////////
function animation() {

  counter ++;
  if( counter>45 ) return;

  draw_all(counter*100/45);
  if( counter == 45 ){
    disp_message_add_complete();
  } else {
    disp_message_adding();
  }
  return;

}

// ========================= ボタン制御 ============================

////////////////////////////////
// 次数によってボタンを有効にしたり無効にしたり
////////////////////////////////
function order_button_check() {

  if( order > 300 ){
    enable_next(false);
  } else {
    enable_next(true);
  }

  if( order <= 1 ){
    enable_prev(false);
  } else {
    enable_prev(true);
  }

  if( order + 5 > 300 ){
    enable_ff(false);
  } else {
    enable_ff(true);
  }

  if( order <= 5 ){
    enable_rew(false);
  } else {
    enable_rew(true);
  }

}

////////////////////////////////
// ボタン確認
////////////////////////////////
function check_buttons () {

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

  if( f_enable_next ){
    if( f_over_next ){
      if( !is_over_next() ){
        show_next_button(0);
        f_over_next = false;
      }
    } else {
      if( is_over_next() ){
        show_next_button(1);
        f_over_next = true;
      }
    }
  }

  if( f_enable_prev ){
    if( f_over_prev ){
      if( !is_over_prev() ){
        show_prev_button(0);
        f_over_prev = false;
      }
    } else {
      if( is_over_prev() ){
        show_prev_button(1);
        f_over_prev = true;
      }
    }
  }

  if( f_enable_ff ){
    if( f_over_ff ){
      if( !is_over_ff() ){
        show_ff_button(0);
        f_over_ff = false;
      }
    } else {
      if( is_over_ff() ){
        show_ff_button(1);
        f_over_ff = true;
      }
    }
  }

  if( f_enable_rew ){
    if( f_over_rew ){
      if( !is_over_rew() ){
        show_rew_button(0);
        f_over_rew = false;
      }
    } else {
      if( is_over_rew() ){
        show_rew_button(1);
        f_over_rew = true;
      }
    }
  }

}

////////////////////////////////
// ボタン作成
////////////////////////////////
function show_buttons() {

  if( is_over_reset() ){
    show_reset_button(1);
    f_over_reset = true;
  } else {
    show_reset_button(0);
    f_over_reset = false;
  }

  if( !f_enable_next ){
    show_next_button(-1);
    f_over_next = false;
  } else if( is_over_next() ){
    show_next_button(1);
    f_over_next = true;
  } else {
    show_next_button(0);
    f_over_next = false;
  }

  if( f_enable_prev ){
    if( is_over_prev() ){
      show_prev_button(1);
      f_over_prev = true;
    } else {
      show_prev_button(0);
      f_over_prev = false;
    }
  } else {
    show_prev_button(-1);
    f_over_prev = false;
  }

  if( f_enable_ff ){
    if( is_over_ff() ){
      show_ff_button(1);
      f_over_ff = true;
    } else {
      show_ff_button(0);
      f_over_ff = false;
    }
  } else {
    show_ff_button(-1);
    f_over_ff = false;
  }

  if( f_enable_rew ){
    if( is_over_rew() ){
      show_rew_button(1);
      f_over_rew = true;
    } else {
      show_rew_button(0);
      f_over_rew = false;
    }
  } else {
    show_rew_button(-1);
    f_over_rew = false;
  }

}

// ボタン位置設定
var btn_y = 450;


var rst_x = 25;
var rst_y = btn_y;
var rst_size_x = 80;
var rst_size_y = 50;

var next_x = 520;
var next_y = btn_y;
var next_size_x = 50;
var next_size_y = 50;

var prev_x = 440;
var prev_y = btn_y;
var prev_size_x = 50;
var prev_size_y = 50;

var ff_x = 280;
var ff_y = btn_y;
var ff_size_x = 50;
var ff_size_y = 50;

var rew_x = 200;
var rew_y = btn_y;
var rew_size_x = 50;
var rew_size_y = 50;

////////////////////////////////
// リセットボタン作成
////////////////////////////////
function show_reset_button(f) {

  ctx.fillStyle = "rgb(100, 100, 100)";
  fillRoundRect(rst_x-3, rst_y-3, rst_size_x+6, rst_size_y+6, 13 );
  if(f){
    ctx.fillStyle = "rgb(255, 150, 150)"; // 光る
  } else {
    ctx.fillStyle = "rgb(255, 90, 90)";
  }
  fillRoundRect(rst_x, rst_y, rst_size_x, rst_size_y, 10 );

  var str;
  ctx.fillStyle = "rgb(255, 255, 255)";
  ctx.font = "bold 16px sans-serif";
  str = "RESET";
  ctx.fillText(str, rst_x + 12, rst_y + 30);

}

////////////////////////////////
// リセットボタンの上にあるか
////////////////////////////////
function is_over_reset(){

  if( mouseX > rst_x && mouseX < rst_x + rst_size_x && mouseY > rst_y && mouseY < rst_y + rst_size_y ){
    return true;
  }
  return false;
}


////////////////////////////////
// 次ボタン作成（1:光る 0:通常 -1:無効)
////////////////////////////////
function show_next_button(f) {

  ctx.fillStyle = "rgb(150, 120, 120)";
  fillRoundRect(next_x-3, next_y-3, next_size_x+6, next_size_y+6, 13 );
  if( f==1 ){
    ctx.fillStyle = "rgb(180, 230, 180)"; // 光る
  } else if( f==0 ){
    ctx.fillStyle = "rgb(125, 180, 90)";
  } else {
    ctx.fillStyle = "rgb(50, 80, 50)"; // 無効
  }
  fillRoundRect(next_x, next_y, next_size_x, next_size_y, 10 );

  var str;

  if( f==-1 ){
    ctx.fillStyle = "rgb(100, 100, 100)"; // 無効
  } else {
    ctx.fillStyle = "rgb(255, 255, 255)";
  }
  ctx.font = "bold 16px sans-serif";
  str = "次";
  ctx.fillText(str, next_x + 16, next_y + 30);

}

////////////////////////////////
// 次ボタンの上にあるか
////////////////////////////////
function is_over_next(){

  if( mouseX > next_x && mouseX < next_x + next_size_x && mouseY > next_y && mouseY < next_y + next_size_y ){
    return true;
  }
  return false;
}

////////////////////////////////
// 次ボタン有効・無効
////////////////////////////////
function enable_next(f){

  if( f ){
    f_enable_next = true;
    if( is_over_next() ){
      show_next_button(1);
    } else {
      show_next_button(0);
    }
  } else {
    f_enable_next = false;
    show_next_button(-1);
  }

}

////////////////////////////////
// 戻るボタン作成
////////////////////////////////
function show_prev_button(f) {

  ctx.fillStyle = "rgb(150, 120, 120)";
  fillRoundRect(prev_x-3, prev_y-3, prev_size_x+6, prev_size_y+6, 13 );
  if( f==1 ){
    ctx.fillStyle = "rgb(180, 230, 180)"; // 光る
  } else if( f==0 ){
    ctx.fillStyle = "rgb(125, 180, 90)";
  } else {
    ctx.fillStyle = "rgb(50, 80, 50)"; // 無効
  }
  fillRoundRect(prev_x, prev_y, prev_size_x, prev_size_y, 10 );

  var str;
  if( f==-1 ){
    ctx.fillStyle = "rgb(100, 100, 100)"; // 無効
  } else {
    ctx.fillStyle = "rgb(255, 255, 255)";
  }
  ctx.font = "bold 16px sans-serif";
  str = "戻る";
  ctx.fillText(str, prev_x+10, prev_y+30);

}
////////////////////////////////
// 戻るボタンの上にあるか
////////////////////////////////
function is_over_prev(){

  if( mouseX > prev_x && mouseX < prev_x + prev_size_x && mouseY > prev_y && mouseY < prev_y + prev_size_y ){
    return true;
  }
  return false;
}
////////////////////////////////
// 戻るボタン有効・無効
////////////////////////////////
function enable_prev(f){

  if( f ){
    f_enable_prev = true;
    if( is_over_prev() ){
      show_prev_button(1);
    } else {
      show_prev_button(0);
    }
  } else {
    f_enable_prev = false;
    show_prev_button(-1);
  }

}

////////////////////////////////
// 早送りボタン作成
////////////////////////////////
function show_ff_button(f) {

  ctx.fillStyle = "rgb(150, 120, 120)";
  fillRoundRect( ff_x-3, ff_y-3, ff_size_x+6, ff_size_y+6, 13 );
  if( f==1 ){
    ctx.fillStyle = "rgb(150, 150, 230)"; // 光る
  } else if( f==0 ){
    ctx.fillStyle = "rgb(50, 20, 220)";
  } else {
    ctx.fillStyle = "rgb(50, 20, 90)"; // 無効
  }
  fillRoundRect(ff_x, ff_y, ff_size_x, ff_size_y, 10 );

  var str;
  if( f==-1 ){
    ctx.fillStyle = "rgb(80, 80, 80)"; // 無効
  } else {
    ctx.fillStyle = "rgb(255, 255, 255)";
  }
  ctx.font = "bold 16px sans-serif";
  str = "+5";
  ctx.fillText(str, ff_x + 13, ff_y + 30);

}
////////////////////////////////
// 早送りボタンの上にあるか
////////////////////////////////
function is_over_ff(){

  if( mouseX > ff_x && mouseX < ff_x + ff_size_x && mouseY > ff_y && mouseY < ff_y + ff_size_y ){
    return true;
  }
  return false;
}
////////////////////////////////
// 早送りボタン有効・無効
////////////////////////////////
function enable_ff(f){

  if( f ){
    f_enable_ff = true;
    if( is_over_ff() ){
      show_ff_button(1);
    } else {
      show_ff_button(0);
    }
  } else {
    f_enable_ff = false;
    show_ff_button(-1);
  }

}
////////////////////////////////
// 巻き戻しボタン作成
////////////////////////////////
function show_rew_button(f) {

  ctx.fillStyle = "rgb(150, 120, 120)";
  fillRoundRect(rew_x-3, rew_y-3, rew_size_x+6, rew_size_y+6, 13 );
  if( f==1 ){
    ctx.fillStyle = "rgb(150, 150, 230)"; // 光る
  } else if( f==0 ){
    ctx.fillStyle = "rgb(50, 20, 220)";
  } else {
    ctx.fillStyle = "rgb(50, 20, 90)"; // 無効
  }
  fillRoundRect(rew_x, rew_y, rew_size_x, rew_size_y, 10 );

  var str;
  if( f==-1 ){
    ctx.fillStyle = "rgb(80, 80, 80)"; // 無効
  } else {
    ctx.fillStyle = "rgb(255, 255, 255)";
  }
  ctx.font = "bold 16px sans-serif";
  str = "-5";
  ctx.fillText(str, rew_x+15, rew_y+30);
}
////////////////////////////////
// 巻き戻しボタンの上にあるか
////////////////////////////////
function is_over_rew(){

  if( mouseX > rew_x && mouseX < rew_x + rew_size_x && mouseY > rew_y && mouseY < rew_y + rew_size_y ){
    return true;
  }
  return false;
}
////////////////////////////////
// 巻き戻しボタン有効・無効
////////////////////////////////
function enable_rew(f){

  if( f ){
    f_enable_rew = true;
    if( is_over_rew() ){
      show_rew_button(1);
    } else {
      show_rew_button(0);
    }
  } else {
    f_enable_rew = false;
    show_rew_button(-1);
  }

}


// ======================== メッセージ ========================


////////////////////////////////
// 「開始」のメッセージ
////////////////////////////////
function disp_message_start() {

  var str;
  ctx.fillStyle = "rgb(0, 0, 0)";
  ctx.font = "24px sans-serif";
  str = "指やマウスで好きな波形を描いて下さい。";
  ctx.fillText(str, 80, 420);
}

////////////////////////////////
// 「計算中」のメッセージ
////////////////////////////////
function disp_message_cal() {

  var str;
  ctx.fillStyle = "rgb(200, 0, 0)";
  ctx.font = "24px sans-serif";
  str = "計算中・・・";
  ctx.fillText(str, 100, 420);
}

////////////////////////////////
// 「平均」のメッセージ
////////////////////////////////
function disp_message_average() {

  var str;
  ctx.fillStyle = "rgb(0, 0, 0)";
  ctx.font = "24px sans-serif";
  str = "この曲線（赤）の平均はこれくらい（青）です。";
  ctx.fillText(str, 35, 420);
}

////////////////////////////////
// 追加項データの表示
////////////////////////////////
function disp_message_term(n) {

  var str;
  var k;
  var factor;
  var tm;
  var func;

  ctx.fillStyle = "rgb(0, 0, 0)";
  ctx.font = "24px sans-serif";

  str = "[" + n + "項]";
  tm = ctx.measureText(str);
  ctx.fillText(str, 100-tm.width, 420);

  if( n % 2 == 1 ){
    k = (n+1)/2;
    factor = (factor_sin[k]/(graph_height/2)).toFixed(2);
    func = "sin";
  } else {
    k = n/2;
    factor = (factor_cos[k]/(graph_height/2)).toFixed(2);
    func = "cos";
  }

  str = "" + factor + " ×";
  tm = ctx.measureText(str);
  ctx.fillText(str, 200-tm.width, 420);

  tm = ctx.measureText(func);
  ctx.fillText(func, 240-tm.width, 420);

  if( k==1 ){
    str = "θ";
  } else {
    str = "(" + k + " θ)";
  }
  ctx.fillText(str, 245, 420);
}

////////////////////////////////
// 「加えます」のメッセージ
////////////////////////////////
function disp_message_add() {

  var str;
  ctx.fillStyle = "rgb(0, 0, 0)";
  ctx.font = "20px sans-serif";
  str = "これ(緑線)を青線に加えます。";
  ctx.fillText(str, 340, 420);
}

////////////////////////////////
// 合成中のメッセージ
////////////////////////////////
function disp_message_adding() {

  var str;
  var tm;

  ctx.fillStyle = "rgb(0, 0, 0)";
  ctx.font = "24px sans-serif";

  str = "[" + order + "項]";
  tm = ctx.measureText(str);
  ctx.fillText(str, 100-tm.width, 420);

  ctx.fillStyle = "rgb(200, 0, 0)";
  str = "合成中・・・";
  ctx.fillText(str, 150, 420);

}

////////////////////////////////
// 合成完了のメッセージ
////////////////////////////////
function disp_message_add_complete() {

  var str;
  var tm;

  ctx.fillStyle = "rgb(0, 0, 0)";
  ctx.font = "24px sans-serif";

  str = "[" + order + "項]";
  tm = ctx.measureText(str);
  ctx.fillText(str, 100-tm.width, 420);

  ctx.fillStyle = "rgb(200, 0, 0)";
  str = "合成完了";
  ctx.fillText(str, 150, 420);

}

////////////////////////////////
// メッセージを全部消す
////////////////////////////////
function delete_message() {

  ctx.fillStyle = "rgb(255, 255, 255)";
  ctx.fillRect(25,395,600,35);
}



