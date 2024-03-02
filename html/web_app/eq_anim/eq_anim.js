///////////////////////////////////////////////////////////////
// �����A�j���W�F�l���[�^�[
// Copyright (c) 2015 K.Hiroe

// ���ӁF�f�l�����l���^���ō�������̂Ȃ̂ŎQ�l�ɂ��ׂ��łȂ��B


//////////////////////////
// ���ϐ�
//////////////////////////
var width = 640;
var height = 480;
var tmp_w = 640;
var tmp_h = 480;

var ctx;           // canvas�R���e�L�X�g
var timerID;
var running = false;
var end_flag = false;
var startfrom_flag = false;
var startfrom_sec;
var f_timeron = false;

var store = "";

// �f���� whammy.js �g�p
var video;

var commands = [
"zoom",
"pan",
"tilt",
"focus",
"movemode",
"set",
"setr",
"setl",
"telop",
"copy",
"move",
"mvr",
"mvl",
"vanish",
"color",
"deform",
"font",
"group",
"arc",
"fps",
"screen",
"bgcolor",
"end",
"startfrom",
"timeron"
];

var camera;
var actor = [];
var timechart = [];
var lines = [];
var glist = [];
var errmes;
var framerate = 30;
var cur_font = "font1";
var cur_style = "";

var cmd_cnt; // ���s�����R�}���h��
var last_tick; // �I���\��R�}��
var tick; // ��ƒ��R�}��
var tickmsec; // ��R�}���Ƃ̌o�ߎ���(�~���b�j
var first_frame;

//�w�i�F
var bg_r = 255;
var bg_g = 255;
var bg_b = 255;

// �w�i�F�ω����
var bg_coloring = false;
var bg_t1 = 0;
var bg_t2 = 0;
var bg_r_org = 0;
var bg_g_org = 0;
var bg_b_org = 0;
var bg_r_dst = 0;
var bg_g_dst = 0;
var bg_b_dst = 0;

//////////////////////////
// ������
//////////////////////////
onload = function() {

  // ��ʕ\���p�R���e�L�X�g�̎擾
  var canvas = document.getElementById('drawfield');
  if (!canvas || !canvas.getContext) {
    alert("HTML5�Ή��u���E�U�ł݂̂����������܂�");
    return false;
  }

  ctx = canvas.getContext('2d');

  ctx.fillStyle="rgb(255,255,255)";
  ctx.fillRect(0,0,width,height);

  running = false;

  // �t�H���g���m���ɓǂݍ��܂��邽�߁A��x�\������K�v������B
  // ����͎w�肵���t�H���g�ŕ`���Ă���Ȃ��Ƃ������ۂ��������Ă���B
  ctx.font = "100px 'font1'";
  ctx.fillText("abc",0,0);
  ctx.font = "100px 'font2'";
  ctx.fillText("abc",0,0);

}

////////////////////////////////
// �T���v������{�^��
////////////////////////////////
function sample_write() {

  store = scriptform.text.value;
  scriptform.text.value = "set( 0, 3, dog, dog, -0.5, 0, 0.4 ) //�@��\n"
                    + "set( 0, 1, cat, c, -0.25, -0.15, 0.13 )    //  �L\n"
                    + "set( 0, 0, mouse, m, 0.25, 0.4, 0.07 ) //  �l�Y�~\n"
                    + "set( 0, 0, stone, s, 0, 0, 0.07 ) //  ��\n"
                    + "\n"
                    + "//�R�����g������܂�\n"
                    + "move( 0, 6, dog,  1.25, 0 ) // ���ړ�\n"
                    + "move( 1, 6, cat, 0.625, -0.08 )�@// �L�ړ�\n"
                    + "move( 2, 2.5, mouse, 0.1, -1.25 ) //�@�l�Y�~�ړ�\n"
                    + "\n"
                    + "move( 4, 1, cat, -0.3, -0.4 )\n"
                    + "\n"
                    + "color( 4, 1, cat, 255,0,0 )�@// �L�ϐF\n"
                    + "deform(3, 3, mouse, 10, 40 ) // �l�Y�~���剻\n"
                    + "\n"
                    + "//�J��������n\n"
                    + "zoom( 6, 2, 0.333)\n"
                    + "pan( 8, 0.5, 0.1)\n"
                    + "pan( 8.5, 0.5, -0.2)\n"
                    + "pan( 9, 0.5, 0.1)\n"
                    + "tilt( 9.5, 0.5, 0.1)\n"
                    + "tilt( 10, 1, -0.2)\n"
                    + "tilt( 11, 0.5, 0.1)\n"
                    + "focus(11.5, 3, stone, 0.4, 0, -0.25)\n"
                    + "focus(15, 0.7, cat, 0.3, 0, -0.25)\n";

}

function script_clear() {
  store = scriptform.text.value;
  scriptform.text.value = "";
}

function script_restore() {
  scriptform.text.value = store;
}



////////////////////////////////
// ���s�{�^��
////////////////////////////////
function run() {

  if( running ) return;
  running = true;
  scriptform.runbtn.disabled = true;
  scriptform.stopbtn.disabled = false;
  startfrom_flag = false;
  f_timeron = false;

  // ��x��ʂ𔒎���
  ctx.fillStyle = "rgb(255,255,255)";
  ctx.fillRect(0, 0, width, height);

  // �J�n�O�̃R�}���h�`�F�b�N
  var rc = precheck();
  if( rc == -1 ){
    running = false;
    scriptform.runbtn.disabled = false;
    scriptform.stopbtn.disabled = true;
    return;
  }

  // �R�}���h�����n�񏇂Ƀ\�[�g����
  timechart.sort(
    function(a,b){
      if( a.time > b.time ) return 1;
      if( a.time < b.time ) return -1;
      if( a.line > b.line ) return 1;
      return -1;
    }
  );

  errmes = "�X�N���v�g�\���`�F�b�N�E�E�E����\n";

  cmd_cnt = 0;
  tick = 0;
  last_tick = 0;
  actor = [];
  camera = classCAMERA();
  width = tmp_w;
  height = tmp_h;
  end_flag = false;

  if( startfrom_flag ){
    if( startfrom_sec <= 0 ){
      startfrom_flag = false;
    }
  }

  errmes += "�A�j���[�V�����J�n (" + width + "*" + height + "  " + framerate + "fps)\n";
  if( f_timeron ){
    errmes += "�f�o�O�p�^�C�}�[�\������\n";
  }
  if( startfrom_flag ){
    errmes += "startfrom�R�}���h�ɏ]�� " + startfrom_sec + " �b��܂ŃJ�b�g\n";
  }

  error.message.value = errmes;

  var area = document.getElementById('screen');
  area.style.width = width + "px";
  area.style.height = height + "px";

  area = document.getElementById('videoarea');
  area.style.width = width + "px";
  area.style.height = height + "px";

  drawfield.width = width;
  drawfield.height = height;

  videofield.width = width;
  videofield.height = height;


  // �^�C�}�[�C�x���g�֐��̐ݒ�
  tickmsec = 1000/framerate;
  if( startfrom_flag ){
    timerID = setInterval(timerfunc, 4);
    ctx.fillStyle = "rgb(255,255,255)";
    ctx.fillRect(0, 0, width, height);
  } else {
    timerID = setInterval(timerfunc, tickmsec);
  }
  first_frame = true;
  cur_font = "font1";
  cur_style = "";

}

////////////////////////////////
// �A�j���J�n�O�̊ȈՃG���[�`�F�b�N
////////////////////////////////
function precheck() {

  var error_num = 0;
  tmp_w = 640;
  tmp_h = 480;

  framerate = 30;

  errmes = "";
  timechart = [];

  lines = splitlines( scriptform.text.value );

  if( scriptform.text.value == "" ){
    show_message("�R�}���h����������Ŏ��s���Ă�������");
    error.message.value = "�R�}���h����������Ŏ��s���Ă�������";
    return -1;
  }

  for ( i=0; i<lines.length; i++) {
    line = commentout( lines[i] );
	if( line == "" ) continue;
    cmd = get_cmd( line );
    if( cmd == "" ){
      errmes += "���@��̌�肪����܂� (" + (i+1) +"�s��)\n";
      error_num ++;
      continue;
    }
    if( cmd.length >15 ){
      errmes += "��`����Ă��Ȃ��R�}���h�ł��B�R�}���h�������߂��܂� (" + (i+1) +"�s��)\n";
      error_num ++;
      continue;
    }
    if( check_cmd( cmd ) == -1 ){
      errmes += "\"" + cmd +"\"�͒�`����Ă��Ȃ��R�}���h�ł� (" + (i+1) +"�s��)\n";
      error_num ++;
      continue;
    }

    // ���ʃR�}���h�Ȃ̂ŗ�O�I����
    if( cmd == "fps" ){
      var rate = get_argument_float(line,0);
      rate = parseInt(rate);
      if( isNaN(rate) ){
        errmes += "��1�����i�b�ԃt���[�����j�ɂ͐��l�����Ă������� (" + (i+1) +"�s��)\n";
        error_num ++;
        continue;
      }
      if( rate == 0 || rate >60 ){
        errmes += "��1�����i�b�ԃt���[�����j�ɂ� (1�`60) �܂ł̐����l�����Ă������� (" + (i+1) +"�s��)\n";
        error_num ++;
        continue;
      }
      framerate = rate;
      errmes += "�b�ԃt���[������" + framerate + "�ɐݒ肵�܂��� (" + (i+1) +"�s��)\n";
      continue;
    }

    // ���ʃR�}���h�Ȃ̂ŗ�O�I����
    if( cmd == "screen" ){
      var w = get_argument_float(line,0);
      var h = get_argument_float(line,1);
      w = parseInt(w);
      h = parseInt(h);
      if( isNaN(w) ){
        errmes += "��1�����i���h�b�g���j�ɂ͐��l�����Ă������� (" + (i+1) +"�s��)\n";
        error_num ++;
        continue;
      }
      if( isNaN(h) ){
        errmes += "��2�����i�c�h�b�g���j�ɂ͐��l�����Ă������� (" + (i+1) +"�s��)\n";
        error_num ++;
        continue;
      }
      if( w <64 || h <48 ){
        errmes += "�ݒ�ł���X�N���[���T�C�Y�͍ŏ� (64 * 48)�ł� (" + (i+1) +"�s��)\n";
        error_num ++;
        continue;
      }
      if( w >1280 || h >720 ){
        errmes += "�ݒ�ł���X�N���[���T�C�Y�͍ő� (1280 * 720)�ł� (" + (i+1) +"�s��)\n";
        error_num ++;
        continue;
      }
      tmp_w = w;
      tmp_h = h;
      errmes += "��ʃT�C�Y�� ��: " + width + " , �c: " + height + "�ɐݒ肵�܂��� (" + (i+1) +"�s��)\n";
      continue;
    }

    // ���ʃR�}���h�Ȃ̂ŗ�O�I����
    if( cmd == "startfrom" ){
      if( startfrom_flag ){
        errmes += "startfrom �R�}���h����ȏ゠��܂� (" + (i+1) +"�s��)\n";
        error_num ++;
        continue;
      }
      startfrom_flag = true;
      var sec = get_argument_float(line,0);
      if( isNaN(sec) ){
        errmes += "�p�����[�^���s���ł� (" + (i+1) +"�s��)\n";
        error_num ++;
        continue;
      }
      startfrom_sec = sec;
    }

    // ���ʃR�}���h�Ȃ̂ŗ�O�I����
    if( cmd == "timeron" ){
      f_timeron = true;
      continue;
    }

    // �R�}���h��Ƃ��ĔF�߂�ꂽ���̂ɂ��āA���n��\�[�g�̂��߂̃��X�g�쐬
    var t1 = get_argument(line,0);
    if( t1 == "" ){
      errmes += "��1����������܂��� (" + (i+1) +"�s��)\n";
      error_num ++;
      continue;
    }
    t1 = parseFloat(t1);
    if( isNaN(t1) ){
      errmes += "��1�����i�J�n�b�j�ɂ͐��l�����Ă������� (" + (i+1) +"�s��)\n";
      error_num ++;
      continue;
    }
    timechart.push( classTC(i,t1) );
  }

  if( error_num > 0 ){
      show_message("��~�F�G���[������܂�");
      errmes += "���v " + error_num + "�̃G���[������܂�";
      error.message.value = errmes;
      return -1;
  }

  if( timechart.length == 0 ){
    show_message("��~�F�`��R�}���h���܂܂�Ă��܂���");
    errmes += "���ۂɕ`�悳����R�}���h���������܂���B";
    error.message.value = errmes;
    return -1;
  }

  return 0;
}

////////////////////////////////
// �X�N���[���Ƀ��b�Z�[�W
////////////////////////////////
function show_message( str ) {

  ctx.fillStyle = "rgb(255,0,0)";
  ctx.font = "bold 100px serif";
  var met = ctx.measureText(str)
  var w = met.width;

  // �ő��25px���炢�ɂ������B�܂�A100px�ŉ������肵�Ă邩�� ratio < 0.25 �ɂ���B
  // �E����30,30���炢�󂯂����B��������ʂ��������Ȃ��Ă����炻��Ȃɋ󂯂��ɑS�̂�80%���炢�Ɏ��߂�

  var ratio = width * 0.8 /w;
  if( ratio > 0.25 ) ratio = 0.25;
  var padding = width * 0.1;
  if( padding > 30 ) padding = 30;
  ctx.setTransform( ratio,0,0,ratio, width - w*ratio - padding, height - padding );
  ctx.fillText(str,0,0);
  ctx.setTransform(1,0,0,1,0,0);

}

////////////////////////////////
// ��~�{�^��
////////////////////////////////
function stop() {

  close();
  error.message.value = "������~\n";
}


////////////////////////////////
// �^�C�}�[�C�x���g�֐�
////////////////////////////////
function timerfunc() {

  tick ++;

  if( startfrom_flag ){
    if( (tick * tickmsec)/1000 >= startfrom_sec ){
      startfrom_flag = false;
      clearInterval(timerID)
      timerID = setInterval(timerfunc, tickmsec);
    }
  }

  // �R�}���h���S���Ȃ��Ȃ��āA���A�ŏI�b�����߂�����I������B
  if( (tick > last_tick+1) && (cmd_cnt >= timechart.length) ){
    errmes += "�A�j���[�V�����I���i����j\n";
    error.message.value = errmes;
    video_close();
    show_message("�A�j���[�V�����I��");
    return;
  }

  if( end_flag ){
    errmes += "end�R�}���h�ɂ�蒆�r�I�����܂���\n";
    if( first_frame ){
      errmes += "�A�j���J�n�O�ɒ��r�I���������߃r�f�I�͍���Ă��܂���\n";
      close();
    } else {
      video_close();
    }
    error.message.value = errmes;
    show_message("end�R�}���h�ɂ�蒆�r�I�����܂���");
    return;
  }

  // �R�}���h�`�F�b�N
  while( (cmd_cnt < timechart.length ) && (timechart[cmd_cnt].time <= ((tick * tickmsec)/1000) ) ){
     cur_lnum = timechart[cmd_cnt].line;
     rc = cmd_exec( lines[ cur_lnum ] );
     if( rc == -1 ){
       error.message.value = errmes;
       show_message("��~�F�G���[������܂�");
       close();
       return;
     }
     cmd_cnt++;
  }

  // �w�i�F�ω�
  if( bg_coloring ){
    if( (tickmsec*tick)/1000 >= bg_t2 ){
      bg_r = bg_r_dst;
      bg_g = bg_g_dst;
      bg_b = bg_b_dst;
      bg_coloring = false;
    } else {
      var progress = ((tickmsec*tick)/1000-bg_t1)/(bg_t2-bg_t1);
      bg_r = parseInt( (bg_r_dst-bg_r_org) * progress + bg_r_org );
      bg_g = parseInt( (bg_g_dst-bg_g_org) * progress + bg_g_org );
      bg_b = parseInt( (bg_b_dst-bg_b_org) * progress + bg_b_org );
    }
  }

  // �J�����ړ�����
  if( camera.zooming ){
       if( (tickmsec*tick)/1000 >= camera.zt2 ){
         camera.z = camera.zdst;
         camera.zooming = false;
       } else {
         var progress = ((tickmsec*tick)/1000-camera.zt1)/(camera.zt2-camera.zt1);
         camera.z = (camera.zdst-camera.zorg) * progress + camera.zorg;
      }
  }
  if( camera.panning ){
       if( (tickmsec*tick)/1000 >= camera.xt2 ){
         camera.x = camera.xdst;
         camera.panning = false;
       } else {
         var progress = ((tickmsec*tick)/1000-camera.xt1)/(camera.xt2-camera.xt1);
         camera.x = (camera.xdst-camera.xorg) * progress + camera.xorg;
      }
  }
  if( camera.tilting ){
       if( (tickmsec*tick)/1000 >= camera.yt2 ){
         camera.y = camera.ydst;
         camera.tilting = false;
       } else {
         var progress = ((tickmsec*tick)/1000-camera.yt1)/(camera.yt2-camera.yt1);
         camera.y = (camera.ydst-camera.yorg) * progress + camera.yorg;
      }
  }


  for(var key in actor){

    var obj = actor[key];

    if( obj.moving ){
       if( (tickmsec*tick)/1000 >= obj.t2 ){
         obj.x = obj.xdst;
         obj.y = obj.ydst;
         obj.moving = false;
       } else {
         var progress = ((tickmsec*tick)/1000-obj.t1)/(obj.t2-obj.t1);
         obj.x = (obj.xdst-obj.xorg) * progress + obj.xorg;
         obj.y = (obj.ydst-obj.yorg) * progress + obj.yorg;
      }
    }

    if( obj.coloring ){
       if( (tickmsec*tick)/1000 >= obj.colt2 ){
         obj.r = obj.r_dst;
         obj.g = obj.g_dst;
         obj.b = obj.b_dst;
         obj.coloring = false;
         if( obj.vanishing ) obj.exist = false;
       } else {
         var progress = ((tickmsec*tick)/1000-obj.colt1)/(obj.colt2-obj.colt1);
         obj.r = parseInt( (obj.r_dst-obj.r_org) * progress + obj.r_org );
         obj.g = parseInt( (obj.g_dst-obj.g_org) * progress + obj.g_org );
         obj.b = parseInt( (obj.b_dst-obj.b_org) * progress + obj.b_org );
      }
    }

    if( obj.sizing ){
       if( (tickmsec*tick)/1000 >= obj.szt2 ){
         obj.sx = obj.sx_dst;
         obj.sy = obj.sy_dst;
         obj.x += obj.rxdst;
         obj.y += obj.rydst;
         obj.rx = 0;
         obj.ry = 0;
         obj.sizing = false;
       } else {
         var progress = ((tickmsec*tick)/1000-obj.szt1)/(obj.szt2-obj.szt1);
         obj.sx = (obj.sx_dst-obj.sx_org) * progress + obj.sx_org;
         obj.sy = (obj.sy_dst-obj.sy_org) * progress + obj.sy_org;
         obj.rx = obj.rxdst * progress;
         obj.ry = obj.rydst * progress;
      }

    }

    actor[key] = obj;
  }

  if( startfrom_flag ) return;

  // ��x������
  ctx.fillStyle = "rgb(" + bg_r + "," + bg_g + "," + bg_b + ")";
  ctx.fillRect(0, 0, width, height);


  for(var key in actor){

    var obj = actor[key];

    // ���������͕̂`�悵�Ȃ�
    if( obj.exist == false ) continue;

    r = obj.r;
    g = obj.g;
    b = obj.b;
    str = obj.str;

    if( obj.telop ){
      sx = obj.sx;
      sy = obj.sy;
      x = (obj.x + obj.rx) + width/2;
      y = -(obj.y + obj.ry) + height/2;
    } else {
      sx = obj.sx * camera.z;
      sy = obj.sy * camera.z;
      x = (obj.x + obj.rx - camera.x) * camera.z + width/2;
      y = -(obj.y + obj.ry - camera.y) * camera.z + height/2;
    }

    ctx.fillStyle = "rgb(" + r + "," + g + "," + b + ")";
    ctx.font = obj.style + " 100px '" + obj.font + "'";
    ctx.setTransform( sx,0,0,sy, x, y );
    ctx.fillText(str,0,0);
    ctx.setTransform(1,0,0,1,0,0);

  }

  if( first_frame ){
    first_frame = false;
    video = new Whammy.Video(framerate);
  }
  video.add(ctx);

  if( f_timeron ){
    ctx.fillStyle = "rgb(255,0,0)";
    ctx.font = "16px 'font2'";
    var sec = tick * tickmsec/1000;
    str = "" + sec.toFixed(2);
    var met = ctx.measureText(str);
    ctx.fillText(str, width-met.width-12, 20);
  }

}

////////////////////////////////
// �I��
////////////////////////////////
function video_close() {

  close();

  var output = video.compile();
  var url = window.URL.createObjectURL(output);
  document.getElementById('videofield').src = url;

}


////////////////////////////////
// �R�}���h���s
////////////////////////////////
function cmd_exec( line ) {

  var rc;

  var cmd = get_cmd(line);
  switch( cmd ){
    case "zoom": rc = zoom(line);  break;
    case "pan":  rc = pan(line); break;
    case "tilt": rc = tilt(line); break;
    case "focus": rc = focus(line); break;
    case "movemode": rc = movemode(line); break;
    case "set": rc = set(line, false); break;
    case "setr": rc = setr(line); break;
    case "setl": rc = setl(line); break;
    case "telop": rc = set(line, true); break;
    case "copy": rc = copy(line); break;
    case "move": rc = move(line); break;
    case "mvr":  rc = mvr(line); break;
    case "mvl":  rc = mvl(line); break;
    case "vanish": rc = vanish(line); break;
    case "color": rc = color(line); break;
    case "deform": rc = deform(line); break;
    case "font": rc = setfont(line); break;
    case "group": rc = group(line); break;
    case "bgcolor": rc = bgcolor(line); break;
    case "end": rc = 0; end_flag = true; break;
    defalult:
      errmes += "��������Ă��Ȃ��R�}���h\"" + cmd + "\"�𖳎����܂���";
      error.message.value = errmes;
      rc = 0;
  }

  return rc;
}

////////////////////////////////
// �I������
////////////////////////////////
function close() {
    running = false;
    clearInterval(timerID)
    scriptform.runbtn.disabled = false;
    scriptform.stopbtn.disabled = true;

}

////////////////////////////////
// �J�����N���X
////////////////////////////////
function classCAMERA() {
  var obj = new Object();

  // �p�����
  obj.panning = false;
  obj.xt1 = 0;
  obj.xt2 = 0;
  obj.x = 0;
  obj.xorg = 0;
  obj.xdst = 0;

  // �e�B���g���
  obj.tilting = false;
  obj.yt1 = 0;
  obj.yt2 = 0;
  obj.y = 0;
  obj.yorg = 0;
  obj.ydst = 0;

  // �Y�[�����
  obj.zooming = false;
  obj.zt1 = 0;
  obj.zt2 = 0;
  obj.z = 1;
  obj.zorg = 0;
  obj.zdst = 0;

  return obj;
}

////////////////////////////////
// �A�N�^�[�N���X
////////////////////////////////
function classACT( str ) {
  var obj = new Object();

  // ��{���
  obj.exist = true;
  obj.vanishing = false;
  obj.str = str;
  obj.font = cur_font;
  obj.style = cur_style;
  obj.telop = false;

  // �ʒu�ω����
  obj.moving = false;
  obj.t1 = 0;
  obj.t2 = 0;
  obj.x = 0;
  obj.y = 0;
  obj.xorg = 0;
  obj.yorg = 0;
  obj.xdst = 0;
  obj.ydst = 0;

  // �F�ω����
  obj.coloring = false;
  obj.colt1 = 0;
  obj.colt2 = 0;
  obj.r = 0;
  obj.g = 0;
  obj.b = 0;
  obj.r_org = 0;
  obj.g_org = 0;
  obj.b_org = 0;
  obj.r_dst = 0;
  obj.g_dst = 0;
  obj.b_dst = 0;

  // �T�C�Y�ω����
  obj.sizing = false;
  obj.szt1 = 0;
  obj.szt2 = 0;
  obj.sx = 0;
  obj.sy = 0;
  obj.sx_org = 0;
  obj.sy_org = 0;
  obj.sx_dst = 0;
  obj.sy_dst = 0;

  // �T�C�Y�ω��ɔ������Έʒu�ω����
  obj.rx = 0;
  obj.ry = 0;
  obj.rxdst = 0;
  obj.rydst = 0;

  return obj;
}

////////////////////////////////
// �^�C���`���[�g�N���X
////////////////////////////////
function classTC(line, time) {
  var obj = new Object();
  obj.line = line;
  obj.time = time;
  return obj;
}


////////////////////////////////
// �����񕪊�(�u���E�U�ɂ���ċ󕶎���̈������قȂ�炵���̂Ŏ���j
////////////////////////////////
function split( str, delim ) {
  var result = [];
  var index;
  var delimlen = delim.length;

  index = str.indexOf(delim);

  while( index != -1) {
    if(index == 0) result.push("");
    else result.push( str.substr(0,index) );
    str = str.substr( index + delimlen );
    index = str.indexOf(delim);
  }
  result.push(str);
  return result;
}

////////////////////////////////
// ���s�ŕ�������
////////////////////////////////
function splitlines( str ) {

  var result = [];
  var i = 0;
  var top = 0;
  var len = str.length;
  var chr;

  for( i=0; i<len; i++ ){
    chr = str.charAt(i);
    if( chr == '\r' ){
      result.push( str.substring(top, i) );
      if( str.charAt(i+1) == '\n' ) i++;
      top = i+1;
    } else if( chr == '\n' ){
      result.push( str.substring(top, i) );
      top = i+1;
    }
  }
  result.push( str.substring(top, i+1) );

  return result;
}

////////////////////////////////
// �R�����g�L���ȍ~���폜���A�󔒂�����
////////////////////////////////
function commentout( str ) {

  var index;
  var result;

  index = str.indexOf("//");
  if( index == -1 ){
    result = str;
  } else {
    result = str.substr(0,index);
  }
  
  //�󔒂�����
  result = result.replace(/\s+/g, "");

  return result;
}


////////////////////////////////
// �R�}���h���擾
////////////////////////////////
function get_cmd( str ) {

  var index;
  var result;

  index = str.indexOf("(");
  if( index == -1 ){
    result = str;
  } else {
    result = str.substr(0,index);
  }

  // �O��̃X�y�[�X����菜��
  result = result.replace( /(^\s+)|(\s+$)/g, "" );

  return result;
}

////////////////////////////////
// �R�}���h���̑��݊m�F
////////////////////////////////
function check_cmd( str ) {

  var i;

  for( i=0; i<commands.length; i++ ){
    if( str == commands[i] ){
      return 0;
    }
  }
  return -1;
}

////////////////////////////////
// �R�}���h�̑� n ���������o���Bn=0 ���Ƒ�1����
////////////////////////////////
function get_argument( str, n ) {

  var index;
  var result;

  // �J�b�R"("���O�𔍂���
  index = str.indexOf("(");
  str = str.substring( index+1, str.length );

  // �J�b�R")"����𔍂���
  index = str.indexOf(")");
  if( index != -1 ){
    str = str.substr( 0, index );
  }

  // �u��,�v���G�X�P�[�v����
  str = str.replace( /\\,/g, "ichijiokikae" );

  result = split( str, "," );

  if( result[n] == null ) return "";

  str = result[n];
  // ���ɖ߂�
  str = str.replace( /ichijiokikae/g, "," );

  // ���e�̃X�y�[�X���폜
  str = str.replace( /(^\s+)|(\s+$)/g, "" );

  return str;

}

////////////////////////////////
// �R�}���h�̑� n �����𐔒l�����Ď��o���Bn=0 ���Ƒ�1����
////////////////////////////////
function get_argument_float( str, n ) {

  return parseFloat( get_argument(str,n) );
}


////////////////////////////////
// �R�}���h�e��
////////////////////////////////

function zoom( line ) {

  var time = get_argument_float( line,0 );
  var duration = get_argument_float( line,1 );
  var factor = get_argument_float( line,2 );

  if( isNaN(duration) ) duration = 0;
  if( duration<0 ){
    errmes += "���ʎ������Ԃɂ� 0 �ȏ�̒l��ݒ肵�Ă������� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( isNaN(factor) ){ factor = 1; }

  if( factor <= 0 ){
    errmes += "�Y�[���͐��̒l�łȂ���΂����܂��� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }

  var tick = (time + duration ) * 1000 /tickmsec;
  if( tick > last_tick ) last_tick = tick;

  if( duration == 0 ){
    camera.zooming = false;
    camera.z *= factor;
    return 0;
  }

  camera.zooming = true;
  camera.zt1 = time;
  camera.zt2 = time + duration;
  camera.zorg = camera.z;
  camera.zdst = camera.z * factor;
  return 0;
}

////////////////////////////////
function pan( line ) {

  var time = get_argument_float( line,0 );
  var duration = get_argument_float( line,1 );
  var factor = get_argument_float( line,2 );

  if( isNaN(duration) ) duration = 0;
  if( duration<0 ){
    errmes += "���ʎ������Ԃɂ� 0 �ȏ�̒l��ݒ肵�Ă������� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( isNaN(factor) ){ factor = 0; }

  var tick = (time + duration ) * 1000 /tickmsec;
  if( tick > last_tick ) last_tick = tick;

  if( duration == 0 ){
    camera.panning = false;
    camera.x += width * factor/camera.z;
    return 0;
  }

  camera.panning = true;
  camera.xt1 = time;
  camera.xt2 = time + duration;
  camera.xorg = camera.x;
  camera.xdst = camera.x + width * factor/camera.z;
  return 0;
}
////////////////////////////////
function tilt( line ) {

  var time = get_argument_float( line,0 );
  var duration = get_argument_float( line,1 );
  var factor = get_argument_float( line,2 );

  if( isNaN(duration) ) duration = 0;
  if( duration<0 ){
    errmes += "���ʎ������Ԃɂ� 0 �ȏ�̒l��ݒ肵�Ă������� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( isNaN(factor) ){ factor = 0; }

  var tick = (time + duration ) * 1000 /tickmsec;
  if( tick > last_tick ) last_tick = tick;

  if( duration == 0 ){
    camera.tilting = false;
    camera.y -= height * factor/camera.z;
    return 0;
  }

  camera.tilting = true;
  camera.yt1 = time;
  camera.yt2 = time + duration;
  camera.yorg = camera.y;
  camera.ydst = camera.y - height * factor/camera.z;
  return 0;
}
////////////////////////////////
function focus( line ) {

  var time = get_argument_float( line,0 );
  var duration = get_argument_float( line,1 );
  var target = get_argument( line,2 );
  var zoom = get_argument_float( line,3 );
  var pan = get_argument_float( line,4 );
  var tilt = get_argument_float( line,5 );

  if( isNaN(duration) ) duration = 0;
  if( duration<0 ){
    errmes += "���ʎ������Ԃɂ� 0 �ȏ�̒l��ݒ肵�Ă������� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( target == "" ){
    errmes += "��3����(target1)�͕K�{�ł� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( !(target in actor)&& !(target in glist) ){
    errmes += "�w�肳�ꂽ����\"" + target + "\"�͂܂��o�^����Ă��܂��� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }

  if( isNaN(zoom) ){ zoom = 1; }
  if( isNaN(pan) ){ pan = 0; }
  if( isNaN(tilt) ){ tilt = 0; }

  if( zoom <= 0 ){
    errmes += "zoom �͐��l�łȂ���΂����܂��� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }

  var tick = (time + duration ) * 1000 /tickmsec;
  if( tick > last_tick ) last_tick = tick;

  var left;
  var right;
  var top;
  var bottom;
  var met;

  if( target in actor ){
    ctx.font = actor[target].style + " 100px '" + actor[target].font + "'";
    met = ctx.measureText(actor[target].str);
    left = actor[target].x + actor[target].rx;
    right = actor[target].x + actor[target].rx + met.width * actor[target].sx;
    top = actor[target].y + actor[target].ry + 100 * actor[target].sy;
    bottom = actor[target].y + actor[target].ry ;
  } else {
    left = group_left( target );
    right = group_right( target );
    top = group_top( target );
    bottom = group_bottom( target );
  }

  var newz = width * zoom/(right-left);
  var newx = (left + right)/2 + (right-left) * pan;
  var newy = (top + bottom)/2 + (top-bottom) * tilt;

  if( duration == 0 ){
    camera.zooming = false;
    camera.panning = false;
    camera.tilting = false;
    camera.x = newx;
    camera.y = newy;
    camera.z = newz;
    return 0;
  }

  camera.zooming = true;
  camera.zt1 = time;
  camera.zt2 = time + duration;
  camera.zorg = camera.z;
  camera.zdst = newz;

  camera.panning = true;
  camera.xt1 = time;
  camera.xt2 = time + duration;
  camera.xorg = camera.x;
  camera.xdst = newx;

  camera.tilting = true;
  camera.yt1 = time;
  camera.yt2 = time + duration;
  camera.yorg = camera.y;
  camera.ydst = newy;

  return 0;
}


////////////////////////////////
function set( line, telop ) {

  var time = get_argument_float( line,0 );
  var duration = get_argument_float( line,1 );
  var name = get_argument( line,2 );
  var string = get_argument( line,3 );
  var x = get_argument_float( line,4 );
  var y = get_argument_float( line,5 );
  var size = get_argument_float( line,6 );
  var sx = get_argument_float( line,7 );
  var sy = get_argument_float( line,8 );

  if( isNaN(duration) ) duration = 0;
  if( duration<0 ){
    errmes += "���ʎ������Ԃɂ� 0 �ȏ�̒l��ݒ肵�Ă������� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( name == "" ){
    errmes += "��3����(name)�͕K�{�ł� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( name in glist ){
    errmes += "�I�u�W�F�N�g��\"" + name + "\"�͂��łɃO���[�v���Ƃ��Ďg�p����Ă��܂� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }

  if( string == "" ){
    errmes += "��4����(�����w��j�͕K�{�ł� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( isNaN(x) ){ x = 0; }
  if( isNaN(y) ){ y = 0; }
  if( isNaN(size) ){ size = 0.1; }
  if( isNaN(sx) ){ sx = 1; }
  if( isNaN(sy) ){ sy = 1; }

  var tick = (time + duration ) * 1000 /tickmsec;
  if( tick > last_tick ) last_tick = tick;

  var obj = classACT(string);

  obj.x = x*width/2/camera.z + camera.x;
  obj.y = y*height/2/camera.z + camera.y;
  obj.sx = height/2 * size * 0.01 * sx /camera.z;
  obj.sy = height/2 * size * 0.01 * sy /camera.z;

  if( duration == 0 ){
    obj.coloring = false;
    obj.r = 0;
    obj.g = 0;
    obj.b = 0;
  } else {
    obj.coloring = true;
    obj.colt1 = time;
    obj.colt2 = time + duration;
    obj.r = bg_r;
    obj.g = bg_g;
    obj.b = bg_b;
    obj.r_org = bg_r;
    obj.g_org = bg_g;
    obj.b_org = bg_b;
    obj.r_dst = 0;
    obj.g_dst = 0;
    obj.b_dst = 0;
  }

  obj.telop = telop;

  actor[name] = obj;
  return 0;
}

////////////////////////////////
function setr( line ) {

  var time = get_argument_float( line,0 );
  var duration = get_argument_float( line,1 );
  var name = get_argument( line,2 );
  var string = get_argument( line,3 );
  var target = get_argument( line,4 );
  var x = get_argument_float( line,5 );
  var y = get_argument_float( line,6 );
  var sx = get_argument_float( line,7 );
  var sy = get_argument_float( line,8 );

  if( isNaN(duration) ) duration = 0;
  if( duration<0 ){
    errmes += "���ʎ������Ԃɂ� 0 �ȏ�̒l��ݒ肵�Ă������� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( name == "" ){
    errmes += "��3����(name)�͕K�{�ł� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( name in glist ){
    errmes += "�I�u�W�F�N�g��\"" + name + "\"�͂��łɃO���[�v���Ƃ��Ďg�p����Ă��܂� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( string == "" ){
    errmes += "��4����(�����w��j�͕K�{�ł� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( target == "" ){
    errmes += "��5����(��I�u�W�F�N�g�w��j�͕K�{�ł� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( target in glist ){
    errmes += "��I�u�W�F�N�g�w��ɃO���[�v���͎g���܂��� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( !(target in actor) ){
    errmes += "�w�肳�ꂽ����\"" + target + "\"�͂܂��o�^����Ă��܂��� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( isNaN(x) ){ x = 0; }
  if( isNaN(y) ){ y = 0; }
  if( isNaN(sx) ){ sx = 1; }
  if( isNaN(sy) ){ sy = 1; }

  var tick = (time + duration ) * 1000 /tickmsec;
  if( tick > last_tick ) last_tick = tick;

  var obj = classACT(string);

  // ��������
  ctx.font = actor[target].style + " 100px '" + actor[target].font + "'";
  var met = ctx.measureText(actor[target].str);

  obj.x = actor[target].x + met.width * actor[target].sx * (1+x);
  obj.y = actor[target].y + 100 * actor[target].sy * y;
  obj.sx = actor[target].sx * sx;
  obj.sy = actor[target].sy * sy;

  if( duration == 0 ){
    obj.coloring = false;
    obj.r = 0;
    obj.g = 0;
    obj.b = 0;
  } else {
    obj.coloring = true;
    obj.colt1 = time;
    obj.colt2 = time + duration;
    obj.r = bg_r;
    obj.g = bg_g;
    obj.b = bg_b;
    obj.r_org = bg_r;
    obj.g_org = bg_g;
    obj.b_org = bg_b;
    obj.r_dst = 0;
    obj.g_dst = 0;
    obj.b_dst = 0;
  }

  actor[name] = obj;
  return 0;
}

////////////////////////////////
function setl( line ) {

  var time = get_argument_float( line,0 );
  var duration = get_argument_float( line,1 );
  var name = get_argument( line,2 );
  var string = get_argument( line,3 );
  var target = get_argument( line,4 );
  var x = get_argument_float( line,5 );
  var y = get_argument_float( line,6 );
  var sx = get_argument_float( line,7 );
  var sy = get_argument_float( line,8 );

  if( isNaN(duration) ) duration = 0;
  if( duration<0 ){
    errmes += "���ʎ������Ԃɂ� 0 �ȏ�̒l��ݒ肵�Ă������� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( name == "" ){
    errmes += "��3����(name)�͕K�{�ł� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( name in glist ){
    errmes += "�I�u�W�F�N�g��\"" + name + "\"�͂��łɃO���[�v���Ƃ��Ďg�p����Ă��܂� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( string == "" ){
    errmes += "��4����(�����w��j�͕K�{�ł� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( target == "" ){
    errmes += "��5����(��I�u�W�F�N�g�w��j�͕K�{�ł� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( target in glist ){
    errmes += "��I�u�W�F�N�g�w��ɃO���[�v���͎g���܂��� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( !(target in actor) ){
    errmes += "�w�肳�ꂽ����\"" + target + "\"�͂܂��o�^����Ă��܂��� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( isNaN(x) ){ x = 0; }
  if( isNaN(y) ){ y = 0; }
  if( isNaN(sx) ){ sx = 1; }
  if( isNaN(sy) ){ sy = 1; }

  var tick = (time + duration ) * 1000 /tickmsec;
  if( tick > last_tick ) last_tick = tick;

  var obj = classACT(string);

  // ��������
  ctx.font = actor[target].style + " 100px '" + actor[target].font + "'";
  var met = ctx.measureText(actor[target].str);
  var met2 = ctx.measureText( string );

  obj.sx = actor[target].sx * sx;
  obj.sy = actor[target].sy * sy;
  obj.x = actor[target].x - met.width * actor[target].sx * x - met2.width * obj.sx;
  obj.y = actor[target].y + 100 * actor[target].sy * y;

  if( duration == 0 ){
    obj.coloring = false;
    obj.r = 0;
    obj.g = 0;
    obj.b = 0;
  } else {
    obj.coloring = true;
    obj.colt1 = time;
    obj.colt2 = time + duration;
    obj.r = bg_r;
    obj.g = bg_g;
    obj.b = bg_b;
    obj.r_org = bg_r;
    obj.g_org = bg_g;
    obj.b_org = bg_b;
    obj.r_dst = 0;
    obj.g_dst = 0;
    obj.b_dst = 0;
  }

  actor[name] = obj;
  return 0;
}


////////////////////////////////
function copy( line ) {

  var time = get_argument_float( line,0 );
  var duration = get_argument_float( line,1 );
  var name = get_argument( line,2 );
  var target = get_argument( line,3 );

  if( isNaN(duration) ) duration = 0;
  if( duration<0 ){
    errmes += "���ʎ������Ԃɂ� 0 �ȏ�̒l��ݒ肵�Ă������� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( name == "" ){
    errmes += "��3����(name)�͕K�{�ł� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( name in glist ){
    errmes += "�I�u�W�F�N�g��\"" + name + "\"�͂��łɃO���[�v���Ƃ��Ďg�p����Ă��܂� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( target == "" ){
    errmes += "��4����(�R�s�[���I�u�W�F�N�g�w��j�͕K�{�ł� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( target in glist ){
    errmes += "�R�s�[���I�u�W�F�N�g�w��ɃO���[�v���͎g���܂��� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( !(target in actor) ){
    errmes += "�w�肳�ꂽ����\"" + target + "\"�͂܂��o�^����Ă��܂��� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }

  var tick = (time + duration ) * 1000 /tickmsec;
  if( tick > last_tick ) last_tick = tick;

  var src = actor[target];
  var obj = classACT(src.str);

  obj.sx = src.sx;
  obj.sy = src.sy;
  obj.x = src.x + src.rx;
  obj.y = src.y + src.ry;

  obj.font = src.font;
  obj.style = src.style;
  obj.telop = src.telop;

  if( duration == 0 ){
    obj.coloring = false;
    obj.r = src.r;
    obj.g = src.g;
    obj.b = src.b;
  } else {
    obj.coloring = true;
    obj.colt1 = time;
    obj.colt2 = time + duration;
    obj.r = bg_r;
    obj.g = bg_g;
    obj.b = bg_b;
    obj.r_org = bg_r;
    obj.g_org = bg_g;
    obj.b_org = bg_b;
    obj.r_dst = src.r;
    obj.g_dst = src.g;
    obj.b_dst = src.b;
  }

  actor[name] = obj;
  return 0;
}

////////////////////////////////
function move( line ) {

  var time = get_argument_float( line,0 );
  var duration = get_argument_float( line,1 );
  var name = get_argument( line,2 );
  var dx = get_argument_float( line,3 );
  var dy = get_argument_float( line,4 );

  if( isNaN(duration) ) duration = 0;
  if( duration<0 ){
    errmes += "���ʎ������Ԃɂ� 0 �ȏ�̒l��ݒ肵�Ă������� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( name == "" ){
    errmes += "��3����(name)�͕K�{�ł� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( !(name in actor) && !(name in glist) ){
    errmes += "�w�肳�ꂽ����\"" + name + "\"�͂܂��o�^����Ă��܂��� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }

  if( isNaN(dx) ){ dx = 0; }
  if( isNaN(dy) ){ dy = 0; }

  dx *= (width/2/camera.z);
  dy *= (height/2/camera.z);

  var tick = (time + duration ) * 1000 /tickmsec;
  if( tick > last_tick ) last_tick = tick;

  if( name in actor ){
    moveSub( time, duration, name, dx, dy );
    return 0;
  }

  // �O���[�v�Ƃ��ď���
  for( var i=0; i<glist[name].length; i++ ){
    moveSub( time, duration, glist[name][i], dx, dy );
  }
  return 0;
}

function moveSub( time, duration, name, dx, dy ){

  if( duration == 0 ){
    actor[name].moving = false;
    actor[name].x += dx;
    actor[name].y += dy;
    return;
  }

  actor[name].moving = true;
  actor[name].t1 = time;
  actor[name].t2 = time + duration;
  actor[name].xorg = actor[name].x;
  actor[name].yorg = actor[name].y;
  actor[name].xdst = actor[name].x + dx;
  actor[name].ydst = actor[name].y + dy;
  return;
}

////////////////////////////////
function mvr( line ) {

  var time = get_argument_float( line,0 );
  var duration = get_argument_float( line,1 );
  var name = get_argument( line,2 );
  var target = get_argument( line,3 );
  var x = get_argument_float( line,4 );
  var y = get_argument_float( line,5 );

  if( isNaN(duration) ) duration = 0;
  if( duration<0 ){
    errmes += "���ʎ������Ԃɂ� 0 �ȏ�̒l��ݒ肵�Ă������� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( name == "" ){
    errmes += "��3����(name)�͕K�{�ł� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( !(name in actor) && !(name in glist) ){
    errmes += "�w�肳�ꂽ����\"" + name + "\"�͂܂��o�^����Ă��܂��� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( target == "" ){
    errmes += "��4����(��I�u�W�F�N�g�w��j�͕K�{�ł� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( target in glist ){
    errmes += "��I�u�W�F�N�g�w��ɃO���[�v���͎g���܂��� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( !(target in actor) ){
    errmes += "�w�肳�ꂽ����\"" + target + "\"�͂܂��o�^����Ă��܂��� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }

  if( isNaN(x) ){ x = 0; }
  if( isNaN(y) ){ y = 0; }

  var tick = (time + duration ) * 1000 /tickmsec;
  if( tick > last_tick ) last_tick = tick;

  // ��������
  ctx.font = actor[target].style + " 100px '" + actor[target].font + "'";
  var met = ctx.measureText(actor[target].str);
  var len = actor[target].str.length; // �c���͑���Ȃ��̂ŉ����𕶎����Ŋ���

  var newx = actor[target].x + met.width * actor[target].sx * (1+x);
  var newy = actor[target].y + met.width/len * actor[target].sy * y;

  var dx;
  var dy;

  if( name in actor ){
    dx = newx - actor[target].x;
    dy = newy - actor[target].y;
    moveSub( time, duration, name, dx, dy );
    return 0;
  }

  // �O���[�v�Ƃ��ď���
  dx = newx - group_left(name);
  dy = newy - group_bottom(name);

  for( var i=0; i<glist[name].length; i++ ){
    moveSub( time, duration, glist[name][i], dx, dy );
  }
  return 0;
}


////////////////////////////////
function mvl( line ) {

  var time = get_argument_float( line,0 );
  var duration = get_argument_float( line,1 );
  var name = get_argument( line,2 );
  var target = get_argument( line,3 );
  var x = get_argument_float( line,4 );
  var y = get_argument_float( line,5 );

  if( isNaN(duration) ) duration = 0;
  if( duration<0 ){
    errmes += "���ʎ������Ԃɂ� 0 �ȏ�̒l��ݒ肵�Ă������� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( name == "" ){
    errmes += "��3����(name)�͕K�{�ł� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( !(name in actor) && !(name in glist) ){
    errmes += "�w�肳�ꂽ����\"" + name + "\"�͂܂��o�^����Ă��܂��� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( target == "" ){
    errmes += "��4����(��I�u�W�F�N�g�w��j�͕K�{�ł� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( target in glist ){
    errmes += "��I�u�W�F�N�g�w��ɃO���[�v���͎g���܂��� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( !(target in actor) ){
    errmes += "�w�肳�ꂽ����\"" + target + "\"�͂܂��o�^����Ă��܂��� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }

  if( isNaN(x) ){ x = 0; }
  if( isNaN(y) ){ y = 0; }

  var tick = (time + duration ) * 1000 /tickmsec;
  if( tick > last_tick ) last_tick = tick;

  // ��������
  ctx.font = actor[target].style + " 100px '" + actor[target].font + "'";
  var met = ctx.measureText(actor[target].str);
  var len = actor[target].str.length; // �c���͑���Ȃ��̂ŉ����𕶎����Ŋ���

  var my_width;
  var newx;
  var newy;
  var dx;
  var dy;

  if( name in actor ){
    ctx.font = actor[name].style + " 100px '" + actor[name].font + "'";
    var met2 = ctx.measureText( actor[name].str );
    my_width = met2.width * actor[name].sx;
    newx = actor[target].x - met.width * actor[target].sx * x - my_width;
    newy = actor[target].y + met.width/len * actor[target].sy * y;
    dx = newx - actor[name].x;
    dy = newy - actor[name].y;
    moveSub( time, duration, name, dx, dy );
    return 0;
  }

  // �O���[�v�Ƃ��ď���
  my_width = group_right(name) - group_left(name);
  newx = actor[target].x - met.width * actor[target].sx * x - my_width;
  newy = actor[target].y + met.width/len * actor[target].sy * y;
  dx = newx - group_left(name);
  dy = newy - group_bottom(name);

  for( var i=0; i<glist[name].length; i++ ){
    moveSub( time, duration, glist[name][i], dx, dy );
  }
  return 0;

}

////////////////////////////////
function vanish( line ) {

  var time = get_argument_float( line,0 );
  var duration = get_argument_float( line,1 );
  var name = get_argument( line,2 );

  if( isNaN(duration) ) duration = 0;
  if( duration<0 ){
    errmes += "���ʎ������Ԃɂ� 0 �ȏ�̒l��ݒ肵�Ă������� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( name == "" ){
    errmes += "��3����(name)�͕K�{�ł� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( !(name in actor) && !(name in glist) ){
    errmes += "�w�肳�ꂽ����\"" + name + "\"�͂܂��o�^����Ă��܂��� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }

  var tick = (time + duration ) * 1000 /tickmsec;
  if( tick > last_tick ) last_tick = tick;

  if( name in actor ){
    vanishSub( time, duration, name );
    return 0;
  }

  // �O���[�v�Ƃ��ď���
  for( var i=0; i<glist[name].length; i++ ){
    vanishSub( time, duration, glist[name][i] );
  }
  return 0;

}

function vanishSub( time, duration, name ){

  if( duration == 0 ){
    actor[name].coloring = false;
    actor[name].vanishing = false;
    actor[name].r = bg_r;
    actor[name].g = bg_b;
    actor[name].b = bg_b;
    actor[name].exist = false;
    return;
  }

  actor[name].coloring = true;
  actor[name].vanishing = true;
  actor[name].colt1 = time;
  actor[name].colt2 = time + duration;
  actor[name].r_org = actor[name].r;
  actor[name].g_org = actor[name].g;
  actor[name].b_org = actor[name].b;
  actor[name].r_dst = bg_r;
  actor[name].g_dst = bg_g;
  actor[name].b_dst = bg_b;
  return;
}

////////////////////////////////
function color( line ) {

  var time = get_argument_float( line,0 );
  var duration = get_argument_float( line,1 );
  var name = get_argument( line,2 );
  var r = get_argument_float( line,3 );
  var g = get_argument_float( line,4 );
  var b = get_argument_float( line,5 );

  if( isNaN(duration) ) duration = 0;
  if( duration<0 ){
    errmes += "���ʎ������Ԃɂ� 0 �ȏ�̒l��ݒ肵�Ă������� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( name == "" ){
    errmes += "��3����(name)�͕K�{�ł� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( !(name in actor) && !(name in glist) ){
    errmes += "�w�肳�ꂽ����\"" + name + "\"�͂܂��o�^����Ă��܂��� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }

  if( isNaN(r) ){ r = 0; }
  if( isNaN(g) ){ g = 0; }
  if( isNaN(b) ){ b = 0; }

  r = parseInt(r);
  g = parseInt(g);
  b = parseInt(b);

  if( r<0 || r>255 || g<0 || g>255 || b<0 || b>255 ){
    errmes += "�F�w��� 0 �` 255 �̐����łȂ���΂����܂��� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }

  var tick = (time + duration ) * 1000 /tickmsec;
  if( tick > last_tick ) last_tick = tick;

  if( name in actor ){
    colorSub( time, duration, name, r,g,b );
    return 0;
  }

  // �O���[�v�Ƃ��ď���
  for( var i=0; i<glist[name].length; i++ ){
    colorSub( time, duration, glist[name][i], r,g,b );
  }
  return 0;

}

function colorSub( time, duration, name, r,g,b ){

  actor[name].exist = true;
  actor[name].vanishing = false;

  if( duration == 0 ){
    actor[name].coloring = false;
    actor[name].r = r;
    actor[name].g = g;
    actor[name].b = b;
    return;
  }

  actor[name].coloring = true;
  actor[name].colt1 = time;
  actor[name].colt2 = time + duration;
  actor[name].r_org = actor[name].r;
  actor[name].g_org = actor[name].g;
  actor[name].b_org = actor[name].b;
  actor[name].r_dst = r;
  actor[name].g_dst = g;
  actor[name].b_dst = b;
  return;
}

////////////////////////////////
function deform( line ) {

  var time = get_argument_float( line,0 );
  var duration = get_argument_float( line,1 );
  var name = get_argument( line,2 );
  var sx = get_argument_float( line,3 );
  var sy = get_argument_float( line,4 );
  var ox = get_argument_float( line,5 );
  var oy = get_argument_float( line,6 );

  if( isNaN(duration) ) duration = 0;
  if( duration<0 ){
    errmes += "���ʎ������Ԃɂ� 0 �ȏ�̒l��ݒ肵�Ă������� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( name == "" ){
    errmes += "��3����(name)�͕K�{�ł� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( !(name in actor) && !(name in glist) ){
    errmes += "�w�肳�ꂽ����\"" + name + "\"�͂܂��o�^����Ă��܂��� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }

  if( isNaN(sx) ){ sx = 1; }
  if( isNaN(sy) ){ sy = 1; }
  if( isNaN(ox) ){ ox = 0; }
  if( isNaN(oy) ){ oy = 0; }

  var tick = (time + duration ) * 1000 /tickmsec;
  if( tick > last_tick ) last_tick = tick;

  var gx;
  var gy;

  if( name in actor ){
    deformSub( time, duration, name, sx, sy );
    if( ox != 0 || oy != 0 ){
      ctx.font = actor[name].style + " 100px '" + actor[name].font + "'";
      var met = ctx.measureText(actor[name].str);
      var left = actor[name].x;
      var right = actor[name].x + met.width * actor[name].sx;
      var top = actor[name].y + 100 * actor[name].sy;
      var bottom = actor[name].y;
      gx = left + (right - left)*ox;
      gy = bottom + (top - bottom)*oy;
      deformSubRelative( duration, name, gx, gy, sx, sy );
    }
    return 0;
  }

  // �O���[�v�Ƃ��ď���
  //var gx = group_left( name ) + ( group_right( name ) - group_left( name ) ) * ox;
  gx = group_right( name ) * ox + group_left( name ) * (1-ox);
  //var gy = group_bottom( name ) + ( group_top( name ) - group_bottom( name ) ) * oy;
  gy = group_top( name ) * oy + group_bottom( name ) * (1-oy);
  for( var i=0; i<glist[name].length; i++ ){
    deformSub( time, duration, glist[name][i], sx, sy );
    deformSubRelative( duration, glist[name][i], gx, gy, sx, sy );
  }
  return 0;

}

function deformSub( time, duration, name, sx, sy ){

  actor[name].x += actor[name].rx;
  actor[name].y += actor[name].ry;

  actor[name].rx = 0;
  actor[name].ry = 0;

  if( duration == 0 ){
    actor[name].sizing = false;
    actor[name].sx *= sx;
    actor[name].sy *= sy;
    return;
  }

  actor[name].sizing = true;
  actor[name].szt1 = time;
  actor[name].szt2 = time + duration;
  actor[name].sx_org = actor[name].sx;
  actor[name].sy_org = actor[name].sy;
  actor[name].sx_dst = actor[name].sx * sx;
  actor[name].sy_dst = actor[name].sy * sy;
  return;
}

function deformSubRelative( duration, name, gx, gy, sx, sy ){

  // �O���[�v�̍��������_�ɂ������݈ʒu����sx,sy�{�̈ʒu�ɂ܂ňړ�����̂ŁA(sx-1)�{�̋��������ړ�����
  var rx = ( actor[name].x - gx ) * (sx-1);
  var ry = ( actor[name].y - gy ) * (sy-1);

  if( duration == 0 ){
    actor[name].x += rx;
    actor[name].y += ry;
    return;
  }

  actor[name].rxdst = rx;
  actor[name].rydst = ry;
  return;
}


////////////////////////////////
function setfont( line ) {

  cur_font = get_argument( line,1 );
  cur_style = get_argument( line,2 );

  if( cur_font == "" ){ cur_font = "font1"; }

  return 0;
}


////////////////////////////////
function group( line ) {

  var index;
  var arg;

  // �J�b�R"("���O�𔍂���
  index = line.indexOf("(");
  var str = line.substring( index+1, line.length );

  // �J�b�R")"����𔍂���
  index = str.indexOf(")");
  if( index != -1 ){
    str = str.substr( 0, index );
  }
  arg = split( str, "," );

  if( arg.length < 3 ){
    errmes += "�����̐�������܂��� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }

  var gname = arg[1];
  if( gname == "" ){
    errmes += "��2����(�O���[�v��)�͕K�{�ł� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }
  if( gname in actor ){
    errmes += "�O���[�v��\"" + gname + "\"�͂��łɃI�u�W�F�N�g���Ƃ��Ďg�p����Ă��܂� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }


  var tmp = [];
  var num = 0;

  for( var i=2; i<arg.length; i++ ){
    var name = arg[i];
    if( name in actor ){
      tmp[num] = name;
      num ++;
      continue;
    }
    if( name in glist ){
      for( var j=0; j<glist[name].length; j++ ){
        var found = false;
        for( var k=0; k<tmp.length; k++ ){
          if( tmp[k] == glist[name][j] ){
            found = true;
            break;
          }
        }
        if( found == false ){
          tmp[num] = glist[name][j];
          num++;
        }
      }
      continue;
    }
    errmes += "�w�肳�ꂽ����\"" + name + "\"�͂܂��o�^����Ă��܂��� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }

  glist[gname] = new Array();
  for( i=0; i<tmp.length; i++ ){
    glist[gname][i] = tmp[i];
  }

  return 0;
}

////////////////////////////////
function group_right( gname ) {

  var name = glist[gname][0];
  ctx.font = actor[name].style + " 100px '" + actor[name].font + "'";
  var met = ctx.measureText(actor[name].str);
  var right = actor[name].x + actor[name].rx + met.width * actor[name].sx;

  for(var i=1; i<glist[gname].length; i++){
    name = glist[gname][i];
    ctx.font = actor[name].style + " 100px '" + actor[name].font + "'";
    met = ctx.measureText(actor[name].str);
    if( actor[name].x + actor[name].rx + met.width * actor[name].sx > right ){
      right = actor[name].x + actor[name].rx + met.width * actor[name].sx;
    }
  }
  return right;
}

function group_left( gname ) {

  var name = glist[gname][0];
  var left = actor[name].x + actor[name].rx;

  for(var i=1; i<glist[gname].length; i++){
    name = glist[gname][i];
    if( actor[name].x + actor[name].rx < left ){
      left = actor[name].x + actor[name].rx;
    }
  }
  return left;
}

function group_top( gname ) {

  var name = glist[gname][0];
  var top = actor[name].y + actor[name].ry + 100 * actor[name].sy;

  for(var i=1; i<glist[gname].length; i++){
    name = glist[gname][i];
    if( actor[name].y + actor[name].ry + 100 * actor[name].sy > top ){
      top = actor[name].y + actor[name].ry + 100 * actor[name].sy;
    }
  }
  return top;
}

function group_bottom( gname ) {

  var name = glist[gname][0];
  var bottom = actor[name].y + actor[name].ry;

  for(var i=1; i<glist[gname].length; i++){
    name = glist[gname][i];
    if( actor[name].y + actor[name].ry < bottom ){
      bottom = actor[name].y + actor[name].ry;
    }
  }
  return bottom;
}

////////////////////////////////
function bgcolor( line ) {

  var time = get_argument_float( line,0 );
  var duration = get_argument_float( line,1 );
  var r = get_argument_float( line,2 );
  var g = get_argument_float( line,3 );
  var b = get_argument_float( line,4 );

  if( isNaN(duration) ) duration = 0;

  if( isNaN(r) ){ r = 0; }
  if( isNaN(g) ){ g = 0; }
  if( isNaN(b) ){ b = 0; }

  r = parseInt(r);
  g = parseInt(g);
  b = parseInt(b);

  if( r<0 || r>255 || g<0 || g>255 || b<0 || b>255 ){
    errmes += "�F�w��� 0 �` 255 �̐����łȂ���΂����܂��� (" + (cur_lnum+1) +"�s��)\n";
    return -1;
  }

  var tick = (time + duration ) * 1000 /tickmsec;
  if( tick > last_tick ) last_tick = tick;

  if( duration == 0 ){
    bg_r = r;
    bg_g = g;
    bg_b = b;
    return;
  }

  bg_coloring = true;
  bg_t1 = time;
  bg_t2 = time + duration;
  bg_r_org = bg_r;
  bg_g_org = bg_g;
  bg_b_org = bg_b;
  bg_r_dst = r;
  bg_g_dst = g;
  bg_b_dst = b;

  return 0;
}

