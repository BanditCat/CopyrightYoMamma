headerFontStyle = ovel.fontStyle( 1.7, ovel.color( 0, 0, 1 ), false, false, false ) 

local index = {}
local hw = {}
local helptxt = {}
local helpsb = {}
local helpsearch = ""

local function redrawhw()
  local ta = hw.childArea; ta.x = 0; ta.y = 0;
  local t = helpsb.area
  local ph  
  if t then 
    ph = t.height
  else
    ph = 10
  end
  ta.height = ta.height - ph
  helptxt.area = ta
  ta.y = ta.height
  ta.height = ph
  helpsb.area = ta
  if helpsb.setParts then
    if helpsearch == "" then
      helpsb:setParts( { [ta.width] = "Type into the window to perform a search." } )
    else
      helpsb:setParts( { [ta.width] = helpsearch .. " (press Enter to perform search)" } )
    end
  end
end

local function nullcb( win, mtype, arg1 )
  return true
end

local function hwcb( win, mtype, arg1 )
  if mtype == "close" then
    helptxt.text = ""
    ovel.quit()    --hw:hide()
  elseif mtype == "menu" then
  elseif mtype == "move" then
  elseif mtype == "keydown" then
    if arg1.name == "Enter" then
      show( helpsearch )
      helpsearch = ""
    elseif arg1.name == "Backspace" and helpsearch ~= "" then
      helpsearch = helpsearch :sub( 1, -2 )
    elseif arg1.char then
      helpsearch = helpsearch .. arg1.char
    end
    redrawhw()
  elseif mtype == "resize" or mtype == "maximize" or mtype == "activate" then
    redrawhw()
  elseif mtype == "mouse" and arg1.type == "mousewheel" then    helptxt:relativeScroll( arg1.delta * -4 )  end
  return false
end  

hw = ovel.newwindow( 100, 100, 800, 600, "Help", "toolbox", hwcb )
helptxt = ovel.newwidget( hw, 0, 0, hw.childArea.width, hw.childArea.height, "textNowrap", hwcb )
helpsb = ovel.newwidget( hw, 0, 0, 0, 0, "statusbar", nullcb )


local function renderHelp( wid, text )
  wid:append( text )
end

entities = {}
do 
local ents = { { "quot", 34 }, { "amp", 38 }, { "apos", 39 }, { "lt", 60 }, { "gt", 62 }, { "nbsp", 160 }, { "iexcl", 161 }, { "cent", 162 }, { "pound", 163 }, { "curren", 164 }, { "yen", 165 }, { "brvbar", 166 }, { "sect", 167 }, { "uml", 168 }, { "copy", 169 }, { "ordf", 170 }, { "laquo", 171 }, { "not", 172 }, { "shy", 173 }, { "reg", 174 }, { "macr", 175 }, { "deg", 176 }, { "plusmn", 177 }, { "sup2", 178 }, { "sup3", 179 }, { "acute", 180 }, { "micro", 181 }, { "para", 182 }, { "middot", 183 }, { "cedil", 184 }, { "sup1", 185 }, { "ordm", 186 }, { "raquo", 187 }, { "frac14", 188 }, { "frac12", 189 }, { "frac34", 190 }, { "iquest", 191 }, { "Agrave", 192 }, { "Aacute", 193 }, { "Acirc", 194 }, { "Atilde", 195 }, { "Auml", 196 }, { "Aring", 197 }, { "AElig", 198 }, { "Ccedil", 199 }, { "Egrave", 200 }, { "Eacute", 201 }, { "Ecirc", 202 }, { "Euml", 203 }, { "Igrave", 204 }, { "Iacute", 205 }, { "Icirc", 206 }, { "Iuml", 207 }, { "ETH", 208 }, { "Ntilde", 209 }, { "Ograve", 210 }, { "Oacute", 211 }, { "Ocirc", 212 }, { "Otilde", 213 }, { "Ouml", 214 }, { "times", 215 }, { "Oslash", 216 }, { "Ugrave", 217 }, { "Uacute", 218 }, { "Ucirc", 219 }, { "Uuml", 220 }, { "Yacute", 221 }, { "THORN", 222 }, { "szlig", 223 }, { "agrave", 224 }, { "aacute", 225 }, { "acirc", 226 }, { "atilde", 227 }, { "auml", 228 }, { "aring", 229 }, { "aelig", 230 }, { "ccedil", 231 }, { "egrave", 232 }, { "eacute", 233 }, { "ecirc", 234 }, { "euml", 235 }, { "igrave", 236 }, { "iacute", 237 }, { "icirc", 238 }, { "iuml", 239 }, { "eth", 240 }, { "ntilde", 241 }, { "ograve", 242 }, { "oacute", 243 }, { "ocirc", 244 }, { "otilde", 245 }, { "ouml", 246 }, { "divide", 247 }, { "oslash", 248 }, { "ugrave", 249 }, { "uacute", 250 }, { "ucirc", 251 }, { "uuml", 252 }, { "yacute", 253 }, { "thorn", 254 }, { "yuml", 255 }, { "OElig", 338 }, { "oelig", 339 }, { "Scaron", 352 }, { "scaron", 353 }, { "Yuml", 376 }, { "fnof", 402 }, { "circ", 710 }, { "tilde", 732 }, { "Alpha", 913 }, { "Beta", 914 }, { "Gamma", 915 }, { "Delta", 916 }, { "Epsilon", 917 }, { "Zeta", 918 }, { "Eta", 919 }, { "Theta", 920 }, { "Iota", 921 }, { "Kappa", 922 }, { "Lambda", 923 }, { "Mu", 924 }, { "Nu", 925 }, { "Xi", 926 }, { "Omicron", 927 }, { "Pi", 928 }, { "Rho", 929 }, { "Sigma", 931 }, { "Tau", 932 }, { "Upsilon", 933 }, { "Phi", 934 }, { "Chi", 935 }, { "Psi", 936 }, { "Omega", 937 }, { "alpha", 945 }, { "beta", 946 }, { "gamma", 947 }, { "delta", 948 }, { "epsilon", 949 }, { "zeta", 950 }, { "eta", 951 }, { "theta", 952 }, { "iota", 953 }, { "kappa", 954 }, { "lambda", 955 }, { "mu", 956 }, { "nu", 957 }, { "xi", 958 }, { "omicron", 959 }, { "pi", 960 }, { "rho", 961 }, { "sigmaf", 962 }, { "sigma", 963 }, { "tau", 964 }, { "upsilon", 965 }, { "phi", 966 }, { "chi", 967 }, { "psi", 968 }, { "omega", 969 }, { "thetasym", 977 }, { "upsih", 978 }, { "piv", 982 }, { "ensp", 8194 }, { "emsp", 8195 }, { "thinsp", 8201 }, { "zwnj", 8204 }, { "zwj", 8205 }, { "lrm", 8206 }, { "rlm", 8207 }, { "ndash", 8211 }, { "mdash", 8212 }, { "lsquo", 8216 }, { "rsquo", 8217 }, { "sbquo", 8218 }, { "ldquo", 8220 }, { "rdquo", 8221 }, { "bdquo", 8222 }, { "dagger", 8224 }, { "Dagger", 8225 }, { "bull", 8226 }, { "hellip", 8230 }, { "permil", 8240 }, { "prime", 8242 }, { "Prime", 8243 }, { "lsaquo", 8249 }, { "rsaquo", 8250 }, { "oline", 8254 }, { "frasl", 8260 }, { "euro", 8364 }, { "image", 8465 }, { "weierp", 8472 }, { "real", 8476 }, { "trade", 8482 }, { "alefsym", 8501 }, { "larr", 8592 }, { "uarr", 8593 }, { "rarr", 8594 }, { "darr", 8595 }, { "harr", 8596 }, { "crarr", 8629 }, { "lArr", 8656 }, { "uArr", 8657 }, { "rArr", 8658 }, { "dArr", 8659 }, { "hArr", 8660 }, { "forall", 8704 }, { "part", 8706 }, { "exist", 8707 }, { "empty", 8709 }, { "nabla", 8711 }, { "isin", 8712 }, { "notin", 8713 }, { "ni", 8715 }, { "prod", 8719 }, { "sum", 8721 }, { "minus", 8722 }, { "lowast", 8727 }, { "radic", 8730 }, { "prop", 8733 }, { "infin", 8734 }, { "ang", 8736 }, { "and", 8743 }, { "or", 8744 }, { "cap", 8745 }, { "cup", 8746 }, { "int", 8747 }, { "there4", 8756 }, { "sim", 8764 }, { "cong", 8773 }, { "asymp", 8776 }, { "ne", 8800 }, { "equiv", 8801 }, { "le", 8804 }, { "ge", 8805 }, { "sub", 8834 }, { "sup", 8835 }, { "nsub", 8836 }, { "sube", 8838 }, { "supe", 8839 }, { "oplus", 8853 }, { "otimes", 8855 }, { "perp", 8869 }, { "sdot", 8901 }, { "lceil", 8968 }, { "rceil", 8969 }, { "lfloor", 8970 }, { "rfloor", 8971 }, { "lang", 9001 }, { "rang", 9002 }, { "loz", 9674 }, { "spades", 9824 }, { "clubs", 9827 }, { "hearts", 9829 }, { "diams", 9830 } }
for i = 1, #ents do entities[ ents[ i ][ 1 ] ] = ovel.UTF32To8( ents[ i ][ 2 ] ) end 
endentities[ "emsp" ] = "  "entities[ "ensp" ] = "  "entities[ "thinsp" ] = " "block = { ["code"] = true, ["cite"] = true, ["div"] = true, ["dl"] = true, ["p"] = true, ["pre"] = true, ["h1"] = true, ["h2"] = true, ["h3"] = true, ["h4"] = true, ["h5"] = true, ["h6"] = true }local fn = "luaman.html"local tsth = ovel.loadFile( [[..\docs\]] .. fn )if tsth == nil then tsth = ovel.loadFile( [[docs\]] .. fn ) end--print( ovel.hexAsciiEncode( tsth ) .. "\n\n\n\n" )local v, abouttxt = ovel.expatParseXML( tsth )if not v then  ovel.die( abouttxt )end--print( ovel.hexAsciiEncode( abouttxt[ 2 ][ 10 ][ 1 ] ) )function renderXhtml( win, wid, html )  local stack = { html }  local depth = 1  local i = { 1 }  local str = ""  local render = false  local context = {{ font = ovel.sansserifFontStyle }}  local function render( ending )    local cc = context[ depth ]    if cc.head and cc.title then      win.title = str    elseif cc.hr or cc.br then      if cc.hr then wid:replaceSelectionWithHR( ovel.color( 0.2, 0.2, 0.2 ), 0.5 ) end      wid:append"\n"    else       if not cc.pre then        str = str:gsub( "%s+", " " )      end      if cc.block then        str = str:gsub( "^ ?(.*) ?$", "%1" )        if not cc.middle then           str = "\n" .. str          if not cc.middle then cc.middle = true end        end        if ending then str = str .. "\n" end      end      wid:append( str, cc.font )     end    str = ""  end  while true do    local cur = stack[ depth ][ i[ depth ] ]    if not cur then      render( true )	      depth = depth - 1	if depth == 0 then break end      i[ depth ] = i[ depth ] + 1    elseif type( cur ) == "string" then      str = str .. stack[ depth ][ i[ depth ] ]      i[ depth ] = i[ depth ] + 1    elseif type( cur ) == "table" then      if cur.type == "entity" then        if entities[ cur.entity ] then          str = str .. entities[ cur.entity ]        else          error( "Unrecognized entity " .. cur.entity .. " in html.", 2 )        end        i[ depth ] = i[ depth ] + 1      elseif cur.type == "element" then        render( false )        i[ depth + 1 ] = 1        stack[ depth + 1 ] = cur        depth = depth + 1        local cc = {}        context[ depth ] = cc        for k, v in pairs( context[ depth - 1 ] ) do cc[ k ] = v end        cc.font = ovel.fontStyle( cc.font )        cc.block = nil        if cur.element == "em" then           cc.font.italic = true        elseif cur.element == "strong" then           cc.font.bold = true        elseif cur.element == "pre" then           cc.font.font = "monospace"        elseif cur.element:find( "^h[1-6]$" ) then          cc.font.size = 2 - ( tonumber( cur.element:sub( 2, -1 ) ) / 6 )          cc.font.bold = true        end        if block[ cur.element ] then cc.block = true end        cc[ cur.element ] = true      end    end  endendprint( "\nstart" )renderXhtml( hw, helptxt, abouttxt )print( "\nend" )helptxt:scroll( 0 ) hw:show()while true do
  local itcm = ovel.itcRetrieve()
  if itcm then
    local c = 16
    while itcm and c ~= 0 do
      loop( itcm )
      c = c - 1
      itcm = ovel.itcRetrieve()
    end
  end
  if ovel.isIdle() then
    ovel.sleep( 5 )
  else
    ovel.loop()
  end
end