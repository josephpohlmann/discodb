local ffi = require("ffi")
local ddblib = ffi.load("libdiscodb.so", true)  -- Load lib into global
local discodb = require("discodb")
local db = discodb.open("/tmp/myths.ddb")
local iter = db:get("Roman")
if iter then
    for i = 1,#iter do
        print(i, iter:next())
    end
else
    print("key does not exist")
end
