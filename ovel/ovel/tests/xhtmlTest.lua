--BUGBUG outmoded

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

hw = ovel.newwindow( 100, 100, 400, 300, "Help", "toolbox", hwcb )
helptxt = ovel.newwidget( hw, 0, 0, hw.childArea.width, hw.childArea.height, "textNowrap", hwcb )
helpsb = ovel.newwidget( hw, 0, 0, 0, 0, "statusbar", nullcb )


local function renderHelp( wid, text )
  wid:append( text )
end

 
local v, abouttxt = ovel.expatParseXML( ovel.processGlobals.manual_luaman )if not v then  ovel.die( abouttxt )end--print( ovel.recursiveTostring( abouttxt ) )entities = {}entities[ "quot" ] = ovel.UTF32To8( 34 )
entities[ "amp" ] = ovel.UTF32To8( 38 )
entities[ "apos" ] = ovel.UTF32To8( 39 )
entities[ "lt" ] = ovel.UTF32To8( 60 )
entities[ "gt" ] = ovel.UTF32To8( 62 )
entities[ "nbsp" ] = ovel.UTF32To8( 160 )
entities[ "iexcl" ] = ovel.UTF32To8( 161 )
entities[ "cent" ] = ovel.UTF32To8( 162 )
entities[ "pound" ] = ovel.UTF32To8( 163 )
entities[ "curren" ] = ovel.UTF32To8( 164 )
entities[ "yen" ] = ovel.UTF32To8( 165 )
entities[ "brvbar" ] = ovel.UTF32To8( 166 )
entities[ "sect" ] = ovel.UTF32To8( 167 )
entities[ "uml" ] = ovel.UTF32To8( 168 )
entities[ "copy" ] = ovel.UTF32To8( 169 )
entities[ "ordf" ] = ovel.UTF32To8( 170 )
entities[ "laquo" ] = ovel.UTF32To8( 171 )
entities[ "not" ] = ovel.UTF32To8( 172 )
entities[ "shy" ] = ovel.UTF32To8( 173 )
entities[ "reg" ] = ovel.UTF32To8( 174 )
entities[ "macr" ] = ovel.UTF32To8( 175 )
entities[ "deg" ] = ovel.UTF32To8( 176 )
entities[ "plusmn" ] = ovel.UTF32To8( 177 )
entities[ "sup2" ] = ovel.UTF32To8( 178 )
entities[ "sup3" ] = ovel.UTF32To8( 179 )
entities[ "acute" ] = ovel.UTF32To8( 180 )
entities[ "micro" ] = ovel.UTF32To8( 181 )
entities[ "para" ] = ovel.UTF32To8( 182 )
entities[ "middot" ] = ovel.UTF32To8( 183 )
entities[ "cedil" ] = ovel.UTF32To8( 184 )
entities[ "sup1" ] = ovel.UTF32To8( 185 )
entities[ "ordm" ] = ovel.UTF32To8( 186 )
entities[ "raquo" ] = ovel.UTF32To8( 187 )
entities[ "frac14" ] = ovel.UTF32To8( 188 )
entities[ "frac12" ] = ovel.UTF32To8( 189 )
entities[ "frac34" ] = ovel.UTF32To8( 190 )
entities[ "iquest" ] = ovel.UTF32To8( 191 )
entities[ "Agrave" ] = ovel.UTF32To8( 192 )
entities[ "Aacute" ] = ovel.UTF32To8( 193 )
entities[ "Acirc" ] = ovel.UTF32To8( 194 )
entities[ "Atilde" ] = ovel.UTF32To8( 195 )
entities[ "Auml" ] = ovel.UTF32To8( 196 )
entities[ "Aring" ] = ovel.UTF32To8( 197 )
entities[ "AElig" ] = ovel.UTF32To8( 198 )
entities[ "Ccedil" ] = ovel.UTF32To8( 199 )
entities[ "Egrave" ] = ovel.UTF32To8( 200 )
entities[ "Eacute" ] = ovel.UTF32To8( 201 )
entities[ "Ecirc" ] = ovel.UTF32To8( 202 )
entities[ "Euml" ] = ovel.UTF32To8( 203 )
entities[ "Igrave" ] = ovel.UTF32To8( 204 )
entities[ "Iacute" ] = ovel.UTF32To8( 205 )
entities[ "Icirc" ] = ovel.UTF32To8( 206 )
entities[ "Iuml" ] = ovel.UTF32To8( 207 )
entities[ "ETH" ] = ovel.UTF32To8( 208 )
entities[ "Ntilde" ] = ovel.UTF32To8( 209 )
entities[ "Ograve" ] = ovel.UTF32To8( 210 )
entities[ "Oacute" ] = ovel.UTF32To8( 211 )
entities[ "Ocirc" ] = ovel.UTF32To8( 212 )
entities[ "Otilde" ] = ovel.UTF32To8( 213 )
entities[ "Ouml" ] = ovel.UTF32To8( 214 )
entities[ "times" ] = ovel.UTF32To8( 215 )
entities[ "Oslash" ] = ovel.UTF32To8( 216 )
entities[ "Ugrave" ] = ovel.UTF32To8( 217 )
entities[ "Uacute" ] = ovel.UTF32To8( 218 )
entities[ "Ucirc" ] = ovel.UTF32To8( 219 )
entities[ "Uuml" ] = ovel.UTF32To8( 220 )
entities[ "Yacute" ] = ovel.UTF32To8( 221 )
entities[ "THORN" ] = ovel.UTF32To8( 222 )
entities[ "szlig" ] = ovel.UTF32To8( 223 )
entities[ "agrave" ] = ovel.UTF32To8( 224 )
entities[ "aacute" ] = ovel.UTF32To8( 225 )
entities[ "acirc" ] = ovel.UTF32To8( 226 )
entities[ "atilde" ] = ovel.UTF32To8( 227 )
entities[ "auml" ] = ovel.UTF32To8( 228 )
entities[ "aring" ] = ovel.UTF32To8( 229 )
entities[ "aelig" ] = ovel.UTF32To8( 230 )
entities[ "ccedil" ] = ovel.UTF32To8( 231 )
entities[ "egrave" ] = ovel.UTF32To8( 232 )
entities[ "eacute" ] = ovel.UTF32To8( 233 )
entities[ "ecirc" ] = ovel.UTF32To8( 234 )
entities[ "euml" ] = ovel.UTF32To8( 235 )
entities[ "igrave" ] = ovel.UTF32To8( 236 )
entities[ "iacute" ] = ovel.UTF32To8( 237 )
entities[ "icirc" ] = ovel.UTF32To8( 238 )
entities[ "iuml" ] = ovel.UTF32To8( 239 )
entities[ "eth" ] = ovel.UTF32To8( 240 )
entities[ "ntilde" ] = ovel.UTF32To8( 241 )
entities[ "ograve" ] = ovel.UTF32To8( 242 )
entities[ "oacute" ] = ovel.UTF32To8( 243 )
entities[ "ocirc" ] = ovel.UTF32To8( 244 )
entities[ "otilde" ] = ovel.UTF32To8( 245 )
entities[ "ouml" ] = ovel.UTF32To8( 246 )
entities[ "divide" ] = ovel.UTF32To8( 247 )
entities[ "oslash" ] = ovel.UTF32To8( 248 )
entities[ "ugrave" ] = ovel.UTF32To8( 249 )
entities[ "uacute" ] = ovel.UTF32To8( 250 )
entities[ "ucirc" ] = ovel.UTF32To8( 251 )
entities[ "uuml" ] = ovel.UTF32To8( 252 )
entities[ "yacute" ] = ovel.UTF32To8( 253 )
entities[ "thorn" ] = ovel.UTF32To8( 254 )
entities[ "yuml" ] = ovel.UTF32To8( 255 )
entities[ "OElig" ] = ovel.UTF32To8( 338 )
entities[ "oelig" ] = ovel.UTF32To8( 339 )
entities[ "Scaron" ] = ovel.UTF32To8( 352 )
entities[ "scaron" ] = ovel.UTF32To8( 353 )
entities[ "Yuml" ] = ovel.UTF32To8( 376 )
entities[ "fnof" ] = ovel.UTF32To8( 402 )
entities[ "circ" ] = ovel.UTF32To8( 710 )
entities[ "tilde" ] = ovel.UTF32To8( 732 )
entities[ "Alpha" ] = ovel.UTF32To8( 913 )
entities[ "Beta" ] = ovel.UTF32To8( 914 )
entities[ "Gamma" ] = ovel.UTF32To8( 915 )
entities[ "Delta" ] = ovel.UTF32To8( 916 )
entities[ "Epsilon" ] = ovel.UTF32To8( 917 )
entities[ "Zeta" ] = ovel.UTF32To8( 918 )
entities[ "Eta" ] = ovel.UTF32To8( 919 )
entities[ "Theta" ] = ovel.UTF32To8( 920 )
entities[ "Iota" ] = ovel.UTF32To8( 921 )
entities[ "Kappa" ] = ovel.UTF32To8( 922 )
entities[ "Lambda" ] = ovel.UTF32To8( 923 )
entities[ "Mu" ] = ovel.UTF32To8( 924 )
entities[ "Nu" ] = ovel.UTF32To8( 925 )
entities[ "Xi" ] = ovel.UTF32To8( 926 )
entities[ "Omicron" ] = ovel.UTF32To8( 927 )
entities[ "Pi" ] = ovel.UTF32To8( 928 )
entities[ "Rho" ] = ovel.UTF32To8( 929 )
entities[ "Sigma" ] = ovel.UTF32To8( 931 )
entities[ "Tau" ] = ovel.UTF32To8( 932 )
entities[ "Upsilon" ] = ovel.UTF32To8( 933 )
entities[ "Phi" ] = ovel.UTF32To8( 934 )
entities[ "Chi" ] = ovel.UTF32To8( 935 )
entities[ "Psi" ] = ovel.UTF32To8( 936 )
entities[ "Omega" ] = ovel.UTF32To8( 937 )
entities[ "alpha" ] = ovel.UTF32To8( 945 )
entities[ "beta" ] = ovel.UTF32To8( 946 )
entities[ "gamma" ] = ovel.UTF32To8( 947 )
entities[ "delta" ] = ovel.UTF32To8( 948 )
entities[ "epsilon" ] = ovel.UTF32To8( 949 )
entities[ "zeta" ] = ovel.UTF32To8( 950 )
entities[ "eta" ] = ovel.UTF32To8( 951 )
entities[ "theta" ] = ovel.UTF32To8( 952 )
entities[ "iota" ] = ovel.UTF32To8( 953 )
entities[ "kappa" ] = ovel.UTF32To8( 954 )
entities[ "lambda" ] = ovel.UTF32To8( 955 )
entities[ "mu" ] = ovel.UTF32To8( 956 )
entities[ "nu" ] = ovel.UTF32To8( 957 )
entities[ "xi" ] = ovel.UTF32To8( 958 )
entities[ "omicron" ] = ovel.UTF32To8( 959 )
entities[ "pi" ] = ovel.UTF32To8( 960 )
entities[ "rho" ] = ovel.UTF32To8( 961 )
entities[ "sigmaf" ] = ovel.UTF32To8( 962 )
entities[ "sigma" ] = ovel.UTF32To8( 963 )
entities[ "tau" ] = ovel.UTF32To8( 964 )
entities[ "upsilon" ] = ovel.UTF32To8( 965 )
entities[ "phi" ] = ovel.UTF32To8( 966 )
entities[ "chi" ] = ovel.UTF32To8( 967 )
entities[ "psi" ] = ovel.UTF32To8( 968 )
entities[ "omega" ] = ovel.UTF32To8( 969 )
entities[ "thetasym" ] = ovel.UTF32To8( 977 )
entities[ "upsih" ] = ovel.UTF32To8( 978 )
entities[ "piv" ] = ovel.UTF32To8( 982 )
entities[ "ensp" ] = ovel.UTF32To8( 8194 )
entities[ "emsp" ] = ovel.UTF32To8( 8195 )
entities[ "thinsp" ] = ovel.UTF32To8( 8201 )
entities[ "zwnj" ] = ovel.UTF32To8( 8204 )
entities[ "zwj" ] = ovel.UTF32To8( 8205 )
entities[ "lrm" ] = ovel.UTF32To8( 8206 )
entities[ "rlm" ] = ovel.UTF32To8( 8207 )
entities[ "ndash" ] = ovel.UTF32To8( 8211 )
entities[ "mdash" ] = ovel.UTF32To8( 8212 )
entities[ "lsquo" ] = ovel.UTF32To8( 8216 )
entities[ "rsquo" ] = ovel.UTF32To8( 8217 )
entities[ "sbquo" ] = ovel.UTF32To8( 8218 )
entities[ "ldquo" ] = ovel.UTF32To8( 8220 )
entities[ "rdquo" ] = ovel.UTF32To8( 8221 )
entities[ "bdquo" ] = ovel.UTF32To8( 8222 )
entities[ "dagger" ] = ovel.UTF32To8( 8224 )
entities[ "Dagger" ] = ovel.UTF32To8( 8225 )
entities[ "bull" ] = ovel.UTF32To8( 8226 )
entities[ "hellip" ] = ovel.UTF32To8( 8230 )
entities[ "permil" ] = ovel.UTF32To8( 8240 )
entities[ "prime" ] = ovel.UTF32To8( 8242 )
entities[ "Prime" ] = ovel.UTF32To8( 8243 )
entities[ "lsaquo" ] = ovel.UTF32To8( 8249 )
entities[ "rsaquo" ] = ovel.UTF32To8( 8250 )
entities[ "oline" ] = ovel.UTF32To8( 8254 )
entities[ "frasl" ] = ovel.UTF32To8( 8260 )
entities[ "euro" ] = ovel.UTF32To8( 8364 )
entities[ "image" ] = ovel.UTF32To8( 8465 )
entities[ "weierp" ] = ovel.UTF32To8( 8472 )
entities[ "real" ] = ovel.UTF32To8( 8476 )
entities[ "trade" ] = ovel.UTF32To8( 8482 )
entities[ "alefsym" ] = ovel.UTF32To8( 8501 )
entities[ "larr" ] = ovel.UTF32To8( 8592 )
entities[ "uarr" ] = ovel.UTF32To8( 8593 )
entities[ "rarr" ] = ovel.UTF32To8( 8594 )
entities[ "darr" ] = ovel.UTF32To8( 8595 )
entities[ "harr" ] = ovel.UTF32To8( 8596 )
entities[ "crarr" ] = ovel.UTF32To8( 8629 )
entities[ "lArr" ] = ovel.UTF32To8( 8656 )
entities[ "uArr" ] = ovel.UTF32To8( 8657 )
entities[ "rArr" ] = ovel.UTF32To8( 8658 )
entities[ "dArr" ] = ovel.UTF32To8( 8659 )
entities[ "hArr" ] = ovel.UTF32To8( 8660 )
entities[ "forall" ] = ovel.UTF32To8( 8704 )
entities[ "part" ] = ovel.UTF32To8( 8706 )
entities[ "exist" ] = ovel.UTF32To8( 8707 )
entities[ "empty" ] = ovel.UTF32To8( 8709 )
entities[ "nabla" ] = ovel.UTF32To8( 8711 )
entities[ "isin" ] = ovel.UTF32To8( 8712 )
entities[ "notin" ] = ovel.UTF32To8( 8713 )
entities[ "ni" ] = ovel.UTF32To8( 8715 )
entities[ "prod" ] = ovel.UTF32To8( 8719 )
entities[ "sum" ] = ovel.UTF32To8( 8721 )
entities[ "minus" ] = ovel.UTF32To8( 8722 )
entities[ "lowast" ] = ovel.UTF32To8( 8727 )
entities[ "radic" ] = ovel.UTF32To8( 8730 )
entities[ "prop" ] = ovel.UTF32To8( 8733 )
entities[ "infin" ] = ovel.UTF32To8( 8734 )
entities[ "ang" ] = ovel.UTF32To8( 8736 )
entities[ "and" ] = ovel.UTF32To8( 8743 )
entities[ "or" ] = ovel.UTF32To8( 8744 )
entities[ "cap" ] = ovel.UTF32To8( 8745 )
entities[ "cup" ] = ovel.UTF32To8( 8746 )
entities[ "int" ] = ovel.UTF32To8( 8747 )
entities[ "there4" ] = ovel.UTF32To8( 8756 )
entities[ "sim" ] = ovel.UTF32To8( 8764 )
entities[ "cong" ] = ovel.UTF32To8( 8773 )
entities[ "asymp" ] = ovel.UTF32To8( 8776 )
entities[ "ne" ] = ovel.UTF32To8( 8800 )
entities[ "equiv" ] = ovel.UTF32To8( 8801 )
entities[ "le" ] = ovel.UTF32To8( 8804 )
entities[ "ge" ] = ovel.UTF32To8( 8805 )
entities[ "sub" ] = ovel.UTF32To8( 8834 )
entities[ "sup" ] = ovel.UTF32To8( 8835 )
entities[ "nsub" ] = ovel.UTF32To8( 8836 )
entities[ "sube" ] = ovel.UTF32To8( 8838 )
entities[ "supe" ] = ovel.UTF32To8( 8839 )
entities[ "oplus" ] = ovel.UTF32To8( 8853 )
entities[ "otimes" ] = ovel.UTF32To8( 8855 )
entities[ "perp" ] = ovel.UTF32To8( 8869 )
entities[ "sdot" ] = ovel.UTF32To8( 8901 )
entities[ "lceil" ] = ovel.UTF32To8( 8968 )
entities[ "rceil" ] = ovel.UTF32To8( 8969 )
entities[ "lfloor" ] = ovel.UTF32To8( 8970 )
entities[ "rfloor" ] = ovel.UTF32To8( 8971 )
entities[ "lang" ] = ovel.UTF32To8( 9001 )
entities[ "rang" ] = ovel.UTF32To8( 9002 )
entities[ "loz" ] = ovel.UTF32To8( 9674 )
entities[ "spades" ] = ovel.UTF32To8( 9824 )
entities[ "clubs" ] = ovel.UTF32To8( 9827 )
entities[ "hearts" ] = ovel.UTF32To8( 9829 )
entities[ "diams" ] = ovel.UTF32To8( 9830 )sa = ""function renderXhtml( win, wid, item, context )  context = context or {}  if type( item ) == "string" then    if context.head and context.title then       win.title = item    elseif context.p then      wid:append( "\n\n" )      wid:append( item )    else      wid:append( item )    end  else    if item.type == "element" then      if item.element == "hr" then        wid:replaceSelectionWithHR( ovel.color( 0.7, 0.7, 0.7 ), 0.7 )      else        local str = ""        local tmp = context[ item.element ]        context[ item.element ] = context[ item.element ] and context[ item.element ] + 1 or 1        for i = 1, #item do          if type( item[ i ] ) == "string" then            str = str .. item[ i ]          elseif item[ i ].type == "entity" and entities[ item[ i ].entity ] then            str = str .. entities[ item[ i ].entity ]          else            renderXhtml( win, wid, str, context )            str = ""            renderXhtml( win, wid, item[ i ], context )          end        end        if str ~= "" then          renderXhtml( win, wid, str, context )        end        context[ item.element ] = tmp      end    end  endendrenderXhtml( hw, helptxt, abouttxt )helptxt:scroll( 0 ) hw:show()while true do
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