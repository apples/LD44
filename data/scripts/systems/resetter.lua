local engine = require('engine')

local resetter = {}

function resetter.visit()
    if keys.reset_pressed then
        reset()
    end
end

return resetter
