codes = "ghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#$%^&*()-_=+[]{}|;':,.<>?`~"

if #codes ~= 85 then ovel.die"Should be exactly 85 codes" end

cc = {}

for i = 0,255 do cc[ i ] = 255 end

for i = 1,85 do
  cc[ codes:byte( i ) ] = i - 1
end
print"static u8 pvlAsciiEncodeCodes[] = { "

for i = 1,85 do
  print( codes:byte( i ) .. ( i == 85 and " };\n" or ", " ) )
end

print"static u8 pvlAsciiDecodeCodes[] = { "

for i = 0,255 do
  print( cc[ i ] .. ( i == 255 and " };\n" or ", " ) )
end


