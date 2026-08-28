-- mandelbrot membership count (deterministic checksum variant of CLBG)
local N = tonumber(arg[1]) or 200
local inset, checksum = 0, 0
for y = 0, N-1 do
  local ci = 2.0*y/N - 1.0
  for x = 0, N-1 do
    local cr = 2.0*x/N - 1.5
    local zr, zi, i = 0.0, 0.0, 0
    while i < 50 and zr*zr + zi*zi <= 4.0 do
      zr, zi = zr*zr - zi*zi + cr, 2.0*zr*zi + ci
      i = i + 1
    end
    if i == 50 then inset = inset + 1 end
    checksum = (checksum + i*(x+y)) % 1000000007
  end
end
print("inset " .. inset .. " checksum " .. checksum)
