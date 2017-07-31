--[[-----------------------------------------------------------
log1(string.lower("ABCDEFGHIJKLMNOPQRSTUVWXYZ"))
log1(string.lower("ÀÁÂÃÄÅ¨ÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞ‗"))
log1(string.upper("abcdefghijklmnopqrstuvwxyz"))
log1(string.upper("אבגדהו¸זחטיךכלםמןנסעףפץצקרשתûü‎‏ÿ"))
--]]--[[-------------------------------------------------------
local ebt = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
local rbt = "ÀÁÂÃÄÅ¨ÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞ‗"
local emt = "abcdefghijklmnopqrstuvwxyz"
local rmt = "אבגדהו¸זחטיךכלםמןנסעףפץצקרשתûü‎‏ÿ"
log1(ebt:lower())
log1(rbt:lower())
log1(emt:upper())
log1(rmt:upper())
--]]-----------------------------------------------------------
_krodin_utils.spawn_item_in_inv("treasure_item")