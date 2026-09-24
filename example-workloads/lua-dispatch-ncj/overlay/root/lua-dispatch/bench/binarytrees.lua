-- binary-trees (CLBG style), allocation/GC heavy
local function bottomup(depth)
  if depth > 0 then
    return { bottomup(depth-1), bottomup(depth-1) }
  end
  return {}
end
local function check(t)
  if #t == 2 then return 1 + check(t[1]) + check(t[2]) end
  return 1
end
local N = tonumber(arg[1]) or 12
local mindepth, maxdepth = 4, N
local stretch = maxdepth + 1
print(string.format("stretch tree of depth %d\t check: %d", stretch, check(bottomup(stretch))))
local longlived = bottomup(maxdepth)
for d = mindepth, maxdepth, 2 do
  local iters = 2 ^ (maxdepth - d + mindepth)
  local c = 0
  for _ = 1, iters do c = c + check(bottomup(d)) end
  print(string.format("%d\t trees of depth %d\t check: %d", iters, d, c))
end
print(string.format("long lived tree of depth %d\t check: %d", maxdepth, check(longlived)))
