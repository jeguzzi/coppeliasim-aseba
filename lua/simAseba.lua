local simAseba={}

if loadPlugin then
	simAseba = loadPlugin 'simAseba';
	(require 'simAseba-typecheck')(simAseba)
end

return simAseba
