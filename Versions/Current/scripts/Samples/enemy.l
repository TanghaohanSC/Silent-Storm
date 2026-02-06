perkAddToHit = { per = 10 }

function perkAddToHit.OnFire(a)
 out("perkAddToHit:OnFire ",a)
end

function OnFire(a,b)
 out("Global:OnFire ",a," ",b)
end

function OnHit()
 out("Global:OnHit")
end

-- format MessagesToSet = { <mesage_name> = <function_name>, ... }
MessagesToSet = { OnHit = OnHit, OnFire = { perkAddToHit.OnFire, OnFire } }