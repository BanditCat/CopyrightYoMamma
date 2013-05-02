// study holography to maybe help do vvvvv

// MOST IMPORTANT: develope good/fast/general displacment mapping.

// finish spherehit, make plane, then texture after implenting render to texture.

// make archives and some request functionality.


// OMG BINGO GENERAL RT:  good general flat displacment mapping generelizes very easily using cube maps.

// RIGHT NOW: make flat plane and then make pretty sphereflake demo using ^^^

// Does flash capture mouse? have good sound? communicate with js well?

// makeShader returns a object


// make shaders seperate files
// write a function like generateProcTex( height, width, aStringWithAshader )
// Changing to hybrid raycast/vertex shader scheme

// Sky/displaced ground pass
// hybrid raycast/vertex shader pass



jsg = function(){

// Constants
var dbg = true; //BUGBUG not true for ship
var contextOpts = { antialias: true, depth: false, alpha: false };
var RateDamping = 0.333;
var OverWatchInterval = 200;
var FrameCountWarmup = 200;  // ignore this many frmaes before ascertaining max/min frame times.
var TicksPerFrame = 3;  // will be at least this many tpf, but may fluctuate
var DesiredFramesPerSecond = 120;  // will render everytick that FPS is less than this.
var maxTickCallbacks = 10;
var tickTimeoutInterval = 0;
var devKeyCombo = { 16: 16, 17: 17, 192: 192 };

// Input
var keys = [];
var buttons = [];
var mx = 0;
var my = 0; // Mouse pos in client space
var mw = 0; // Mouse wheel, this can be wrote with no ill effect.

// Rendering
var cv;
var wglc;
var rayTraceVertexBuf;
// Shader stuff
var rayTracePrg;
var vaPosition; 

// Render state
var cameraPos = { x: 0.0, y: 0.0, z: 1.0 };
var cameraDir = { x: 0.0, y: 0.0, z: 1.0 };

// Timing globals
var startTime = new Date();
var overWatchCount = 0;
var tickCount = 0;
var frameCount = 0;
var slidingFps = DesiredFramesPerSecond;
var slidingTps = DesiredFramesPerSecond *  TicksPerFrame;
var minFT = 1000000;
var maxFT = 0;

// Global callbacks
var overwatches = [];
var postRenderCallbacks = [];


// Other
var intervals = [];
var started = false;
var devc = document.getElementById( "jsgDevConsole" );
var devConsoleShown = false;

// Text globals
var devConCSS = [ 
'<style type="text/css">',
'body{',
'  padding: 0px;',
'}',
'canvas{',
'  float: left;',
'  position: absolute;',
'  left: 0px;',
'  top: 0px;',
'}',
'#jsgDevConsole{',
'  float: right;',
'  font-size: 75%;',
'  width: 42%;',
'}',
'.right{',
'  float: right;',
'}',
'</style>' ].join( "\n" );

  

function getConsoleInfo(){
  var keyString = ""; 
  for( i = 0; i < 256; ++i ) 
    if( keys[ i ] ) keyString += ( i + "," ); 
  if( keyString != "" )
    keyString = keyString.slice( 0, -1 );
  var buttonString = ""; 
  for( i = 0; i < 256; ++i ) 
    if( buttons[ i ] ) buttonString += ( i + "," ); 
  if( buttonString != "" )
    buttonString = buttonString.slice( 0, -1 );

  return [
  '<strong>Timing Information</strong>',
  'Sliding Ticks Per Second:  ' + jsg.tps().toFixed( 2 ),
  'Sliding Frames Per Second: ' + jsg.fps().toFixed( 2 ),
  'Overwatches Per Second:    ' + jsg.owps().toFixed( 2 ),
  'Timing Callbacks:          ' + jsg.timingCallbacks() + ' (max ' + jsg.maxTimingCallbacks() + ')',
  'Minimum Frame Time:        ' + jsg.minFrameTime().toFixed( 6 ),
  'Maximum Frame Time:        ' + jsg.maxFrameTime().toFixed( 6 ),
  'Total Frames Rendered:     ' + jsg.totalFrames(),
  '\n<strong>Window Information</strong>',
  'Window size:               ' + window.innerWidth + 'x' + window.innerHeight,
  'Canvas size:               ' + cv.width + 'x' + cv.height,
  '\n<strong>Input Information</strong>',
  'Keys Pressed:              ' + keyString,
  'Buttons Pressed:           ' + buttonString,
  'Mouse Wheel Position:      ' + mw,
  '\n<strong>Camera Information</strong>',
  'Camera Position:           x:' + cameraPos.x.toFixed( 2 ) + ", y:" + cameraPos.y.toFixed( 2 ) + ", z:" + cameraPos.z.toFixed( 2 ),
  'Camera Direction:          x:' + cameraDir.x.toFixed( 2 ) + ", y:" + cameraDir.y.toFixed( 2 ) + ", z:" + cameraDir.z.toFixed( 2 ) ].join( "\n" );
}
  

function consoleHTML(){
  return devConCSS + "<pre id='jsgDevConInfo'>" +
    getConsoleInfo() +
    "</pre>" +
    "<button onclick='jsg.resetTiming()'>Reset Timing</button>" +
    "<form id='consoleEvalForm' onsubmit=\"try{jsg.evalConsole();}catch(e){jsg.log('<strong>Error: </strong>' + e + '<br/>');} return false;\">" +
    "<input type='text' size='20' id='inpConsole'></input><br/>" +
    "<input value='Evaluate' type='button' onclick='jsg.evalConsole()'></input>" +
    "<input value='Hide Console' type='button' onclick='jsg.hideConsole()'></input>" +
    "<input value='Clear Log' type='button' onclick='jsg.clearLog()'></input>" +
    "</form>" +
    "<pre id='consoleLog'></pre>";
 }

function uptime(){
  var ut = new Date().getTime() - startTime.getTime();
  if( ut != 0.0 )
    return ut / 1000.0;
  else
    return 0.0000001;
}
function fps(){ return slidingFps; }
function tps(){ return slidingTps; }
function owps(){ return 1000.0 / OverWatchInterval; }
function maxFrameTime(){ return maxFT; }
function minFrameTime(){ return minFT; }
function registerOverwatch( f ){ overwatches.push( f ); }
function registerPostRenderCallback( f ){ postRenderCallbacks.push( f ); }
function totalFrames(){ return frameCount; }
function timingCallbacks(){ return intervals.length; }
function showConsole(){ if( !devConsoleShown ){ devc.innerHTML = consoleHTML(); devConsoleShown = true; if( window.onresize ) window.onresize(); } }
function hideConsole(){ if(  devConsoleShown ){ devc.innerHTML = devConCSS; devConsoleShown = false; if( window.onresize ) window.onresize(); } }
function toggleConsole(){ if( devConsoleShown ) hideConsole(); else showConsole(); }
function maxTimingCallbacks(){ return maxTickCallbacks; }


function resetTiming(){
  startTime = new Date();
  overWatchCount = 0;
  tickCount = 0;
  frameCount = 0;  
}
function log( lm ){
  var ll = document.getElementById("consoleLog");
  if( ll != null ) ll.innerHTML += lm;
}
function clearLog( s ){
  var ll = document.getElementById("consoleLog");
  if( ll != null ) ll.innerHTML = ( typeof( s ) == "string" ) ? s : "";
}
function start(){
  if( !started ){
    started = true;
    resetTiming();
    setInterval( overWatch, OverWatchInterval );
    intervals.push( setInterval( tick, tickTimeoutInterval ) );
    for( i = 0; i < 256; ++i ) keys[ i ] = false;
    for( i = 0; i < 5; ++i ) buttons[ i ] = false;

    // Callbacks
    document.onmousemove = document.onmouseout =  function( e ){ if( e.clientX ) mx = e.clientX; if( e.clientY ) my = e.clientY; }
    document.onkeydown = function( e ){ keys[e.keyCode] = true; }
	  document.onkeyup = function( e ){ keys[e.keyCode] = false; }
	  document.onmousedown = function( e ){ buttons[ e.button ] = true; }
	  document.onmouseup = function( e ){ buttons[ e.button ] = false; }
    document.oncontextmenu = function( e ){ return false; }
    {
      function wheel( e ){ 
        var delta = 0;
        if( !e ) e = window.event;
        if( e.wheelDelta ) delta = e.wheelDelta / 120;
        if( window.opera ) delta = -delta;
        if( e.detail ) delta = -e.detail / 3;
        mw += delta;
      }
      document.onmousewheel = wheel;
      if( window.addEventListener ) window.addEventListener( 'DOMMouseScroll', wheel, false );
    }

    cv = document.getElementById( "jsgCanvas" );
    if( cv == null )
      fail( "Unable to get canvas!" )
    try{
      wglc = cv.getContext( "webgl", contextOpts );
      if( wglc == null )
        wglc = cv.getContext( "experimental-webgl", contextOpts );  
      if( wglc == null )
        fail( "Failed to get WebGL context" );
    } catch( e ){
      fail( "Canvas does not support WebGL" );
    }
    if( wglc == null )
      fail( "Browser does not support WebGL" )
    devc = document.getElementById( "jsgDevConsole" );
    showConsole();
    if( !dbg )
      hideConsole();
    jsg.registerOverwatch(function(){ 
      if( devConsoleShown )
        document.getElementById( "jsgDevConInfo" ).innerHTML = getConsoleInfo(); 
      });
   
  }
}



function evalConsole(){
  try{ 
    eval( document.getElementById("inpConsole").value ); 
  }catch( e ){ 
    jsg.log( "<strong>Error: </strong>" + e + "<br/>" ); 
  }
}
function fail( s ){
  for( i = 0; i < intervals; ++i )
    clearInterval( intervals[ i ] );
  document.getElementsByTagName( "body" )[ 0 ].innerHTML = "<h1>Epic Fail:</h1><p>" + s + "</p>";
  started = false;
  throw( s );
}


var overWatch = (function(){
  var otc = tickCount;
  var ofc = frameCount;
  return function(){
    if( started ){
      overWatchCount += 1;
      var tc = tickCount;
      var fc = frameCount;
      if( overWatchCount > 3 ){
        slidingFps = ( ( fc - ofc ) * 1000.0 / OverWatchInterval ) * RateDamping + slidingFps * ( 1.0 - RateDamping );
        slidingTps = ( ( tc - otc ) * 1000.0 / OverWatchInterval ) * RateDamping + slidingTps * ( 1.0 - RateDamping );
        if( slidingTps < DesiredFramesPerSecond * TicksPerFrame && intervals.length < maxTickCallbacks )
          intervals.push( setInterval( tick, tickTimeoutInterval ) );
        else if( ( slidingTps > DesiredFramesPerSecond * TicksPerFrame * 2 ) && ( intervals.length > 0 ) )
          clearInterval( intervals.pop() );
      }
      otc = tc;
      ofc = fc;
      for( i = 0; i < overwatches.length; ++i )
        overwatches[ i ]();
    }
  }
})();
function tick(){
if( started ){
  tickCount += 1;
  if( frameCount < uptime() * DesiredFramesPerSecond )
    render();
}
}

function mouseX(){ return mx; }
function mouseY(){ return my; }


// Takes a vertex src, a fragment src, a list of { uniform-name: numberOfElements } pairs, and the gl.
// This function returns an object with two elements: a integer variable named program that is the gl name of this program
// and a a list of closures named uniforms that has a closure for each uniform.
// On error a error string is returned instead
// INFO: not going to support 2x2 mats, only 4 vecs

function makeShader( vsrc, fsrc, uniforms, gl ) {
  var vshd = gl.createShader( gl.VERTEX_SHADER );
  if( vshd == null )
    return "<strong>Error creating vertex shader!</strong><br/>" + inf;
  gl.shaderSource( vshd, vsrc );
  gl.compileShader( vshd );
  if( !gl.getShaderParameter( vshd, gl.COMPILE_STATUS ) ){
    var inf = gl.getShaderInfoLog( vshd );
    gl.deleteShader( vshd );
    return "<strong>Error compiling vertex shader:</strong><br/>" + inf;
  }
    
  var fshd = gl.createShader( gl.FRAGMENT_SHADER );
  if( fshd == null ){
    gl.deleteShader( vshd )
    return "<strong>Error creating fragment shader!</strong><br/>" + inf;
  }
  gl.shaderSource( fshd, fsrc );
  gl.compileShader( fshd );
  if( !gl.getShaderParameter( fshd, gl.COMPILE_STATUS ) ){
    var inf = gl.getShaderInfoLog( fshd );
    gl.deleteShader( fshd );
    gl.deleteShader( vshd )
    return "<strong>Error compiling fragment shader:</strong><br/>" + inf;
  }

  var prg = gl.createProgram();
  if( prg == null )
    return "<strong>Error creating shader program!</strong><br/>";

  gl.attachShader( prg, vshd );
  gl.attachShader( prg, fshd );
  gl.linkProgram( prg );

  if( !gl.getProgramParameter( prg, gl.LINK_STATUS ) ){
    var inf = gl.getProgramInfoLog( prg );
    gl.deleteProgram( prg );
    return "<strong>Error linking shader program!</strong><br/>" + inf;
  }

  var ans = { program: prg, uniforms: {} };
  for( var j = 0; j < uniforms.length; ++j ){
    var un = uniforms[ j ];
    var ui = gl.getUniformLocation( prg, un );
    if( ui != null )
      ans.uniforms[ un ] = ui;
  }
  return ans;
}


function makeShaderOrDie( vsrc, fsrc, uniforms, gl ){
  var ans = makeShader( vsrc, fsrc, uniforms, gl );
  if( typeof( ans ) == "string" )
    jsg.fail( ans );
  else 
    return ans;
}

// Drawing


render = (function(){
  var osdc = false;
  var omx = mx;
  var omy = my;
  var oft = uptime();
  return function(){
    if( started ){
      if( wglc != null ){
        var tt = uptime();
        var tft = tt - oft;
        var tmx = mx;
        var tmy = my;
        draw( wglc, tft, tmx - omx, tmy - omy );
        frameCount += 1;
        omx = tmx; omy = tmy;
        if( tft > maxFT  && frameCount > FrameCountWarmup ) maxFT = tft;
        if( tft < minFT && tft > 0  && frameCount > FrameCountWarmup ) minFT = tft;
        oft = tt;
        for( i = 0; i < postRenderCallbacks.length; ++i )
          postRenderCallbacks[ i ]();
        if( dbg ){
          var sdc = true;
          for( j in devKeyCombo )
            if( !keys[ j ] )
              sdc = false;
          if( sdc & !osdc )
            toggleConsole();
          osdc = sdc;
        }
      }
    }
  };
})();


var rayTraceVertexSource = [
    "attribute vec2 jsg_Position;",

    "varying vec3 eye;",
    "varying vec4 spherePos[ 4 ];",
    "varying vec3 sphereCol[ 4 ];",
    "varying vec3 light;",
    "varying vec3 cameraRePos;",

    "uniform vec3 cameraPos;",
    "uniform vec3 cameraX;",
    "uniform vec3 cameraY;",
    "uniform vec3 cameraZ;",
    "uniform vec2 screen;",

    "void main() {",
    "  gl_Position = vec4( jsg_Position, 0.0, 1.0 );",
    "  eye = vec3( jsg_Position * screen, 1.3 );",
    "  for( int i = 0; i < 4; ++i ){", 
    "    vec3 v;",
    "    vec3 c;",
    "    if( i == 0 ){ v = vec3( 0.4, -0.2, 0.0 ); c = vec3( 0.6, 0.3, 0.6 ); };",
    "    if( i == 1 ){ v = vec3( 0.0, 0.5, 0.0 ); c = vec3( 0.7, 0.5, 0.3 );  }",
    "    if( i == 2 ){ v = vec3( 0.0, -0.1, 0.5 ); c = vec3( 0.5, 0.1, 0.9 );  }",
    "    if( i == 3 ){ v = vec3( -0.3, -0.2, -0.1 ); c = vec3( 0.25, 1.0, 0.25 );  }",
    "    vec3 sp = v;",// - cameraPos;",
    "    spherePos[ i ].x = dot( sp, cameraX );",
    "    spherePos[ i ].y = dot( sp, cameraY );",
    "    spherePos[ i ].z = dot( sp, cameraZ );",
    "    spherePos[ i ].w = 1.0 - length( v );",
    "    sphereCol[ i ] = c;",
    "    cameraRePos.x = -dot( cameraPos, cameraX );",
    "    cameraRePos.y = -dot( cameraPos, cameraY );",
    "    cameraRePos.z = -dot( cameraPos, cameraZ );",
    "  }",
    "  light.x = dot( vec3( 4.0, -5.0, 3.0 ) - cameraPos, cameraX );",
    "  light.y = dot( vec3( 4.0, -5.0, 3.0 ) - cameraPos, cameraY );",
    "  light.z = dot( vec3( 4.0, -5.0, 3.0 ) - cameraPos, cameraZ );",
    "}" ].join( "\n" );



var fragmentSource = [
    "#ifdef GL_ES", 
    "precision highp float;",
    "#endif",

    "varying vec3 eye;",
    "varying vec4 spherePos[ 4 ];", 
    "varying vec3 sphereCol[ 4 ];",
    "varying vec3 light;",
    "varying vec3 cameraRePos;",

    // returns the xy coordinates of the plane intersection.  it is a plane with a fixed y value faxing upwards toward negative y. p is the origin.
//    "vec2 floorHit( inout eyeToNormal, inout p, in float height );"

    // returns 10000.0 or the depth of the sphere intersection.
    "float sphereHit( inout vec3 eyeToNormal, inout vec3 p, in float radius ){",
    "  vec3 en = normalize( eyeToNormal );",
    "  float denp = dot( en, p );",
    "  vec3 ep = en * denp;",
    "  vec3 dir = ( ep - p );",
    "  float ld = dot( dir, dir );",
    "  float r2 = dot( radius, radius );",
    "  if( ld <= r2 && denp > 0.0 ){",
    "    dir -= en * sqrt( r2 - ld );",
    "    p += dir;", 
    "    eyeToNormal = dir / radius;",
    "    return length( p );",
    "  }else",
    "    return 10000.0;",
    "}",
    
    "bool sphereTest( in vec3 eyeToNormal, in vec3 p, in float radius ){",
    "  vec3 en = normalize( eyeToNormal );",
    "  vec3 ep = en * dot( en, p );",
    "  vec3 dir = ( ep - p );",
    "  return dot( dir, dir ) <= dot( radius, radius );",
    "}",

    "vec4 sphereFlakeHit( inout vec3 eyeToNormal, inout vec3 op, in float radius ){",
    "if( sphereTest( eyeToNormal, op, radius ) ){",
    "  float ndpth = 10000.0;",
    "  vec3 c;",
    "  vec3 etn = eyeToNormal;",
    "  vec3 oop = op;",
    "  for( int j = 0; j < 4; j++ ){",
    "    vec3 np = ( spherePos[ j ].xyz * radius ) + oop;",
    "    vec3 ne = etn;",
    "    float nd = sphereHit( ne, np, spherePos[ j ].w * radius );",
    "    if( nd < ndpth ){",
    "      ndpth = nd;",
    "      op = np;",
    "      eyeToNormal = ne;",
    "      c = sphereCol[ j ];",
    "    }",
    "  }",
    "  return vec4( c, ndpth );",
    "}else",
    "  return vec4( 0.0, 0.0, 0.0, 10000.0 );",
    "}",
    "vec4 sphereFlakeHit2( inout vec3 eyeToNormal, inout vec3 op, in float radius ){",
    "if( sphereTest( eyeToNormal, op, radius ) ){",
    "  float ndpth = 10000.0;",
    "  vec3 etn = eyeToNormal;",
    "  vec3 c;",
    "  vec3 oop = op;",
    "  for( int j = 0; j < 4; j++ ){",
    "    vec3 np = ( spherePos[ j ].xyz * radius ) + oop;",
    "    vec3 ne = etn;",
    "    vec4 nd = sphereFlakeHit( ne, np, spherePos[ j ].w * radius );",
    "    if( nd.w < ndpth ){",
    "      ndpth = nd.w;",
    "      op = np;",
    "      eyeToNormal = ne;",
    "      c = nd.xyz / 2.0 + sphereCol[ j ];",
    "    }",
    "  }",
    "  return vec4( c, ndpth );",
    "}else",
    "  return vec4( 0.0, 0.0, 0.0, 10000.0 );",
    "}",
    "vec4 sphereFlakeHit3( inout vec3 eyeToNormal, inout vec3 op, in float radius ){",
    "if( sphereTest( eyeToNormal, op, radius ) ){",
    "  float ndpth = 10000.0;",
    "  vec3 etn = eyeToNormal;",
    "  vec3 c;",
    "  vec3 oop = op;",
    "  for( int j = 0; j < 4; j++ ){",
    "    vec3 np = ( spherePos[ j ].xyz * radius ) + oop;",
    "    vec3 ne = etn;",
    "    vec4 nd = sphereFlakeHit2( ne, np, spherePos[ j ].w * radius );",
    "    if( nd.w < ndpth ){",
    "      ndpth = nd.w;",
    "      op = np;",
    "      eyeToNormal = ne;",
    "      c = nd.xyz / 3.0 + sphereCol[ j ];",
    "    }",
    "  }",
    "  return vec4( c, ndpth );",
    "}else",
    "  return vec4( 0.0, 0.0, 0.0, 10000.0 );",
    "}",


 



    "uniform vec2 mouse;",

    "void main(){",
    "  vec3 nrml = eye;",
    "  vec3 pnt = vec3( 0.0, mouse.y, 0.0 ) + cameraRePos;",
    "  vec4 ans = sphereFlakeHit3( nrml, pnt, 1.0 + mouse.x );",
    "  if( ans.w < 10000.0 ){",
    "    vec3 rnrml = normalize( reflect( normalize( eye ), nrml ) );",
    "    vec3 rpnt = ( vec3( 0.0, mouse.y, 0.0 ) + cameraRePos ) - ( ans.w * normalize( eye ) );",
    "    vec4 rans = sphereFlakeHit3( rnrml, rpnt, 1.0 + mouse.x );",
    "    vec3 lit = normalize( light - pnt );",
    "    if( rans.w < 10000.0 ){",
    "      vec3 rlit = normalize( light - rpnt );",
    "      gl_FragColor = vec4( dot( lit, nrml ) * ans.xyz * 0.6 + dot( rlit, rnrml ) * rans.xyz * 0.4 + 0.1, 1.0 );",
    "    }else",
    "      gl_FragColor = vec4( dot( lit, nrml ) * ans.xyz + 0.1, 1.0 );",
    "  }else",
    "    gl_FragColor = vec4( vec3( 0.0 ), 1.0 );",
    "}"
    ].join( "\n" );


// Cheesy vector funcs
function vdot( u, v ){ return v.x * u.x + v.y * u.y + v.z * u.z; }
function vmult( u, s ){ return { x: u.x * s, y: u.y * s, z: u.z * s }; }
function vlength( v ){ return Math.sqrt( vdot( v, v ) ); }
function vcross( u, v ){ return { x: u.y * v.z - v.y * u.z, y: u.z * v.x - v.z * u.x, z: u.x * v.y - v.x * u.y }; }
function vnormalize( u ){ return vmult( u, 1 / vlength( u ) ); }
function vlist( u ){ return [ u.x, u.y, u.z ]; }

controlCamera = (function (){
  var rx = 0;
  var ry = 0;
  var rz = 7.0;
  var smx = rx;
  var smy = ry;
  var smz = rz;
  var omw = mw;
  var mmx = 0;
  var mmy = 0;
  var smmx = mmx;
  var smmy = mmy;
  return function ( gl, timeDelta, mdx, mdy ){
    if( mw != omw ){
      rz += ( mw - omw ) * 3;
      omw = mw;
    }
    if( buttons[ 0 ] ){
      rz += mdy / 4.0;
    }else if( buttons[ 2 ] ){
      mmx += mdx * 0.003;
      mmy += mdy * 0.003;
    }else{
      rx += mdx * 0.03;
      ry += mdy * 0.03;
    }
    if( ry > Math.PI ) ry = Math.PI;
    if( ry < 0.0 ) ry = 0.0;
    smx += ( rx - smx ) * timeDelta * 10;
    smy += ( ry - smy ) * timeDelta * 10;
    smz += ( rz - smz ) * timeDelta * 10;

    smmx += ( mmx - smmx ) * timeDelta * 10;
    smmy += ( mmy - smmy ) * timeDelta * 10;

    var sx = Math.sin( smx );
    var cx = Math.cos( smx );
    var sy = Math.sin( smy );
    var cy = Math.cos( smy );
    cameraPos.x = sx * sy * smz; cameraPos.y = cy * smz; cameraPos.z = cx * sy * smz;
  
    cameraDir = vmult( cameraPos, -1.0 / vlength( cameraPos ) );
    var cmx = vnormalize( { x: cameraDir.z, y: 0, z: -1.0 * cameraDir.x } );
    var cmz = vnormalize( { x: cameraDir.x, y: cameraDir.y, z: cameraDir.z } );
    var cmy = vcross( cmx, cmz );
    
    gl.uniform3fv( rayTracePrg.uniforms.cameraPos, vlist( cameraPos ) );
    gl.uniform3fv( rayTracePrg.uniforms.cameraX, ( vlist( cmx ) ) );
    gl.uniform3fv( rayTracePrg.uniforms.cameraY, ( vlist( cmy ) ) );
    gl.uniform3fv( rayTracePrg.uniforms.cameraZ, ( vlist( cmz ) ) );
    
    gl.uniform2f( rayTracePrg.uniforms.mouse, smmx, smmy );
    

  }
})();


function drawInit( gl ){
  
  gl.disable( gl.DEPTH_TEST );

  rayTracePrg = new makeShaderOrDie( rayTraceVertexSource, fragmentSource, [ "cameraX", "cameraPos", "cameraY", "cameraZ", "screen", "mouse" ], gl );
  gl.useProgram( rayTracePrg.program );

  // Make data
  rayTraceVertexBuf = gl.createBuffer();
  var screenQuad = new Float32Array( [ -1.0,-1.0, 1.0,-1.0, 1.0, 1.0,-1.0, 1.0 ] );

  gl.bindBuffer( gl.ARRAY_BUFFER, rayTraceVertexBuf );
  gl.bufferData( gl.ARRAY_BUFFER, screenQuad, gl.STATIC_DRAW );
  
  
  window.onresize = function( e ){ 
    cv.height = window.innerHeight;
    cv.width =  window.innerWidth * ( devConsoleShown ? 0.55 : 1.0 );
    gl.viewport( 0, 0, cv.width, cv.height );
  }
  window.onresize();
  gl.flush();

  gl.enableVertexAttribArray( 0 );
  gl.bindAttribLocation( rayTracePrg.program, 0, "jsg_Position" );

}

var draw = (function(){
  var drawInited = false;
  return function ( gl, timeDelta, mdx, mdy ){
    if( !drawInited ){
      drawInited = true;
      drawInit( gl );
    }
    gl.clearColor( 1.0, 0.0, 0.0, 0.0 );
    gl.clear( gl.COLOR_BUFFER_BIT );
  

    gl.bindBuffer( gl.ARRAY_BUFFER, rayTraceVertexBuf );
    gl.vertexAttribPointer( 0, 2, gl.FLOAT, false, 0, 0 );
  
    controlCamera( gl, timeDelta, mdx, mdy );
    gl.uniform2fv( rayTracePrg.uniforms.screen, [ cv.width / Math.sqrt( cv. width * cv.height ), cv.height / Math.sqrt( cv. width * cv.height ) ] );

    gl.drawArrays( gl.TRIANGLE_FAN, 0, 4 );
    gl.flush();
  }
})();






return {
  start: start,
  fail: fail,
  log: log,
  clearLog: clearLog,
  uptime: uptime,
  fps: fps, tps: tps, owps: owps,
  resetTiming: resetTiming,
  maxFrameTime: maxFrameTime, minFrameTime: minFrameTime, totalFrames: totalFrames,
  timingCallbacks: timingCallbacks, maxTimingCallbacks: maxTimingCallbacks,
  showConsole: showConsole,
  hideConsole: hideConsole,
  toggleConsole: toggleConsole,
  evalConsole: evalConsole,


  registerOverwatch: registerOverwatch,
  registerPostRenderCallback: registerPostRenderCallback,

  mouseX: mouseX, 
  mouseY: mouseY, 
};
}()

