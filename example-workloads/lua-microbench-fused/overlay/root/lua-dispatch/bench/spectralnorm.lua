-- spectral-norm (CLBG classic)
local function A(i,j) return 1.0 / ((i+j)*(i+j+1)/2 + i + 1) end
local function AvTimes(x, n, out, transpose)
  for i = 0, n-1 do
    local s = 0
    for j = 0, n-1 do
      if transpose then s = s + A(j,i)*x[j+1] else s = s + A(i,j)*x[j+1] end
    end
    out[i+1] = s
  end
end
local n = tonumber(arg[1]) or 100
local u, v, w = {}, {}, {}
for i = 1, n do u[i] = 1 end
for _ = 1, 10 do
  AvTimes(u, n, w, false); AvTimes(w, n, v, true)
  AvTimes(v, n, w, false); AvTimes(w, n, u, true)
end
local vBv, vv = 0, 0
for i = 1, n do vBv = vBv + u[i]*v[i]; vv = vv + v[i]*v[i] end
io.write(string.format("%0.9f\n", math.sqrt(vBv/vv)))
