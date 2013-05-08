keys = {}
keys[ 8 ] = "Backspace"
keys[ 9 ] = "Tab"
keys[ 12 ] = "Clear"
keys[ 13 ] = "Enter"
keys[ 16 ] = "Shift"
keys[ 17 ] = "Control"
keys[ 18 ] = "Alt"
keys[ 19 ] = "Pause"
keys[ 27 ] = "Escape"
keys[ 32 ] = "Space"
keys[ 33 ] = "Pageup"
keys[ 34 ] = "Pagedown"
keys[ 35 ] = "End"
keys[ 36 ] = "Home"
keys[ 37 ] = "Left"
keys[ 38 ] = "Up"
keys[ 39 ] = "Right"
keys[ 40 ] = "Down"
keys[ 41 ] = "Select"
keys[ 42 ] = "Print"
keys[ 45 ] = "Insert"
keys[ 46 ] = "Delete"
keys[ 91 ] = "Lsys"
keys[ 92 ] = "Rsys"
keys[ 96 ] = "Numpad0"
keys[ 97 ] = "Numpad1"
keys[ 98 ] = "Numpad2"
keys[ 99 ] = "Numpad3"
keys[ 100 ] = "Numpad4"
keys[ 101 ] = "Numpad5"
keys[ 102 ] = "Numpad6"
keys[ 103 ] = "Numpad7"
keys[ 104 ] = "Numpad8"
keys[ 105 ] = "Numpad9"
keys[ 106 ] = "Multiply"
keys[ 107 ] = "Add"
keys[ 109 ] = "Subtract"
keys[ 111 ] = "Divide"
keys[ 112 ] = "F1"
keys[ 113 ] = "F2"
keys[ 114 ] = "F3"
keys[ 115 ] = "F4"
keys[ 116 ] = "F5"
keys[ 117 ] = "F6"
keys[ 118 ] = "F7"
keys[ 119 ] = "F8"
keys[ 120 ] = "F9"
keys[ 121 ] = "F10"
keys[ 122 ] = "F11"
keys[ 123 ] = "F12"
keys[ 124 ] = "F13"
keys[ 125 ] = "F14"
keys[ 126 ] = "F15"
keys[ 127 ] = "F16"
keys[ 128 ] = "F17"
keys[ 129 ] = "F18"
keys[ 130 ] = "F19"
keys[ 131 ] = "F20"
keys[ 132 ] = "F21"
keys[ 133 ] = "F22"
keys[ 134 ] = "F23"
keys[ 135 ] = "F24"
keys[ 144 ] = "Numlock"
keys[ 145 ] = "Scrollock"

print"#ifndef VISLIB_KEYS_H\n#define VISLIB_KEYS_H\n\n"

for i = 0,255,1 do
  local g = keys[ i ]
  local f = "0x" .. string.format( "%X", i )
  if not g then
    if ( i < 48 or i > 57 ) and ( i < 65 or i > 90 ) then
      g = f
    else       
      g = string.char( i )
    end
  else
    print( "#define vlk" .. f .. " " .. i .. "\n" )
  end
  print( "#define vlk" .. g .. " " .. i .. "\n" )
end

print"\n\nstatic u8* vlKeyNames[ 256 ] = {\n"

for i = 0,255,1 do
  local g = keys[ i ]
  local f = "0x" .. string.format( "%X", i )
  if not g then
    if ( i < 48 or i > 57 ) and ( i < 65 or i > 90 ) then
      g = f
    else       
      g = string.char( i )
    end
  end
  print( "  \"" .. g .. "\",\n" )
end

print"};\n\n#endif //VISLIB_KEYS_H"


