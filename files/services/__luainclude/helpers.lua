local fun1 = function(a, b)
    return a+b
end

local fun2 = function(a, b)
    return fun1(a,b) * fun1(b, a)
end

function gfun3(a, b)
    return a*b
end

local module = { ['fun1'] = fun1, ['fun2'] = fun2 }
return module
