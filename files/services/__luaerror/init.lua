
function func3(n)
    local a = {}
    local b = 1
    local c = a+b*a -- error here
    return c-a
end


function func2(a, b)
    local one = 255
    local two = 552
    local thr = two * one - one/two + func3(two * two)
    return thr * one
end


function func1(a)
    local first = 10
    first = first * (a + func2(a*10, a*100))
    return first/2
end


function subscript(req)
    func1(5)
end