--[[---------------------------------------------------------
local ebt = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
local rbt = "ÀÁÂÃÄÅ¨ÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞ‗"
local emt = "abcdefghijklmnopqrstuvwxyz"
local rmt = "אבגדהו¸זחטיךכלםמןנסעףפץצקרשתûü‎‏ÿ"
log1(ebt:lower())
log1(rbt:lower())
log1(emt:upper())
log1(rmt:upper())
--]]---------------------------------------------------------
class 'A'
function A:__init()
   log1('A init\n')
end
function A:__finalize()
   log1('A finalize\n')
end

class 'B' (A)
function B:__init()
   A.__init(self)
   log1('B init\n')
end
function B:__finalize()
   log1('B finalize\n')
end

do
   local obj = B()
   --super = nil
end
collectgarbage('collect')
