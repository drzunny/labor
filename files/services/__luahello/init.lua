
this_is_counter = 0

local __walk_table = nil
__walk_table = function(t, n)
    local walk_str = ''
    for k, v in pairs(t) do
        print(k, v)
        if type(v) == 'table' then
            walk_str = walk_str .. k .. ' : \n' .. __walk_table(v, k)
        else
            walk_str = walk_str .. k .. ' : ' .. tostring(v)
        end
        walk_str = walk_str .. '\n'
    end
    return walk_str
end


function subscript(req)
    this_is_counter = this_is_counter + 1
    if this_is_counter >= 10 and this_is_counter % 10 == 0 then
        print(__walk_table(req))
        if req.args == nil or req.args.name == nil then
            print('Missing arguments...')
        else
            print('You argument is :' .. req.args.name)
        end
    else
        print('Hi, this message from Lua')
    end
end
