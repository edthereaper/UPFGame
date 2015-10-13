test = {
    id="test",
    interact = function(test,callback)
    			callback:printlua("Esto es una prueba para ver si funciona");
			    callback:test()
			    local vector = vec(0,0,0,0)
			    callback:printvector(vector)
    		end
}

