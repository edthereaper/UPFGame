SLB.using(SLB)

lm = LogicManager();
lm:printAgents();
lm:setNumAgents(100);
lm:printAgents();

--[[print('This is lua')

print(LogicManager)

lm:TeleportPlayer(1,2,3);

val=lm:GetPlayerLife();

print(val);

print('Numero de agentes');
print(lm.numagents);]]