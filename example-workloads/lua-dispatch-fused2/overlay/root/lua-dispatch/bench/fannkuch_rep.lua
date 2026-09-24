-- run fannkuch N times in-process (avoids sh/VDSO in the trace window)
local reps = tonumber(arg[1]) or 4
local n = tonumber(arg[2]) or 9
arg = { tostring(n) }
for _ = 1, reps do
  dofile('bench/fannkuch.lua')
end
