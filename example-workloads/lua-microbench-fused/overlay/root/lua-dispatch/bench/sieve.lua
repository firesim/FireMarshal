-- sieve of eratosthenes, branch/array heavy
local N = tonumber(arg[1]) or 100000
local is = {}
for i = 2, N do is[i] = true end
local count = 0
for i = 2, N do
  if is[i] then
    count = count + 1
    for j = i + i, N, i do is[j] = false end
  end
end
print("primes below " .. N .. ": " .. count)
