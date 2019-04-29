local engine = require('engine')

local gui = {}

function gui.visit()
    gui_state.coins = coins
    gui_state.stage = current_stage
end

return gui
