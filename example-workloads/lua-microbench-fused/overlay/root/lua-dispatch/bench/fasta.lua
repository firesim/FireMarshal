-- fasta-style weighted random selection (CLBG LCG), checksummed
local IM, IA, IC = 139968, 3877, 29573
local seed = 42
local function rand(max)
  seed = (seed * IA + IC) % IM
  return max * seed / IM
end
local codes = "acgtBDHKMNRSVWY"
local probs = {0.27,0.12,0.12,0.27,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02,0.02}
local cum, acc = {}, 0
for i,p in ipairs(probs) do acc = acc + p; cum[i] = acc end
local n = tonumber(arg[1]) or 100000
local counts = {}
for i = 1, #codes do counts[i] = 0 end
for _ = 1, n do
  local r = rand(1.0)
  for i = 1, #cum do
    if r < cum[i] then counts[i] = counts[i] + 1; break end
  end
end
local out = {}
for i = 1, #codes do out[i] = codes:sub(i,i) .. "=" .. counts[i] end
print(table.concat(out, " ") .. " seed=" .. seed)
