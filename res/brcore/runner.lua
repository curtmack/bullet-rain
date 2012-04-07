--[[
    bullet rain
    A bullet hell engine by Curtis Mackie
    
    Distributed under the terms of the MIT license
    See LICENSE.TXT in the svn root directory for more information
]]

--[[
    runner.lua
    Code to implement the system that runs bullet scripts.
]]

--[[
    The bullet script table. Each row has two elements:
      co:   The coroutine to resume the bullet's script (nil if it has no script)
      wait: The number of frames we need to wait before resuming
    stage has the same values, but there's only one stage script.
]]
local bullets = { }
local stage = { }

-- Initializes the table
function init_table()
    for i = 0, 8191, 1 do
        bullets[i] = {co = nil, wait = 0}
    end
    stage.co = nil
    stage.wait = 0
end

-- Adds a bullet to the table
function add_bullet(idx, func)
    -- func is a string, not a function, we need to find the function with that name
    bullets[idx].co   = coroutine.create(_G[func]);
    bullets[idx].wait = 0;
end

-- Sets the stage script
function set_stage(func)
    stage.co   = coroutine.create(_G[func]);
    stage.wait = 0;
end

--[[
    The code for handling bullet scripts is entirely Lua.
    The engine calls the exec_bullet_scripts() function via lua_pcall when it's time
    to handle bullet scripts.
    This function iterates through the list of bullets
    For each one, if the coroutine isn't nil, and the wait is up, it sets the context
    and resumes that bullet's script.
]]

function exec_bullet_scripts()
    -- Loop over all bullets
    for id,bul in pairs(bullets) do
        -- Has the bullet died since we last updated it?
        if not bulletrain.is_bullet_dead(id) then
            -- Is the bullet running a script?
            if bul.co ~= nil then
                -- Are we supposed to wait?
                if bul.wait <= 0 then
                
                    -- Set the context
                    bulletrain.set_bullet_context(id)
                    
                    -- Call the routine, set the wait time
                    local temp,idle = coroutine.resume(bul.co)
                    if idle ~= nil then
                        bul.wait = idle
                    end
                    
                    -- Did the coroutine finish?
                    if coroutine.status(bul.co) == "dead" then
                        bul.co = nil
                        bul.wait = 0
                    end
                    
                else
                    -- Update the wait timer
                    bul.wait = bul.wait - 1
                end
            end -- bul.co ~= nil
        else -- is_bullet_dead(id)
            -- Remove the bullet from our list
            bul.co = nil
            bul.wait = 0
        end -- is_bullet_dead(id)
    end -- for
    
    -- This function goes through and kills bullets that are marked for death
    bulletrain.kill_bullets()
    
    --[[
        Now we run the stage script
        Set the context to nil, this tells the engine to run in stage context
        We want to always do this in case we run into a bug somewhere, a null context
        is better than a corrupt or invalid context
    ]]
    bulletrain.set_bullet_context(nil)
    
    -- Is there a stage script running?
    if stage.co ~= nil then
 
        -- Are we waiting?
        if stage.wait <= 0 then
            
            -- Call the routine, set the wait time
            local temp,idle = coroutine.resume(stage.co)
            if idle ~= nil then
                bul.wait = idle
            end
            
            -- Did the coroutine finish?
            if coroutine.status(stage.co) == "dead" then
                stage.co = nil
                stage.wait = 0
            end
            
        else
            -- Update the wait timer
            stage.wait = stage.wait - 1
        end
        
    end
end