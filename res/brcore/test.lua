--[[
    bullet rain
    A bullet hell engine by Curtis Mackie
    
    Distributed under the terms of the MIT license
    See LICENSE.TXT in the svn root directory for more information
]]

--[[
    test.lua
    The main script for the scripting systest.
]]

-- Reversing bullets
function reversing_bullet()
    local xdelta, ydelta = bulletrain.get_velocity_self()
    xdelta = - xdelta / 30
    ydelta = - ydelta / 30
    for i = 1, 70, 1 do
        bulletrain.accelerate_self_by_rect(xdelta, ydelta)
        coroutine.yield()
    end
end