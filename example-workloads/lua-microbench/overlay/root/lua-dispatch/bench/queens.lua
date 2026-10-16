-- n-queens solution count (recursion/call heavy)
local N = tonumber(arg[1]) or 8
local cols, d1, d2 = {}, {}, {}
local count = 0
local function place(row)
  if row > N then count = count + 1; return end
  for c = 1, N do
    if not cols[c] and not d1[row+c] and not d2[row-c+N] then
      cols[c], d1[row+c], d2[row-c+N] = true, true, true
      place(row+1)
      cols[c], d1[row+c], d2[row-c+N] = nil, nil, nil
    end
  end
end
place(1)
print("queens(" .. N .. ") = " .. count)
