local engine = require('engine')
local stages = require('stages')

config = {
}

board = {}
for i = 1, 15 do
    board[i] = {}
    for j = 1, 15 do
        board[i][j] = {}
    end
end

coins = 0

gui_state = {
    coins = 0,
    stage = 0,
    fps = 0,
    version = ''
}

local stage_list = {
    stages.teamwork,
    stages.winner
}

current_stage = 1

stage_list[1]()

function reset()
    for r,row in ipairs(board) do
        for c,col in ipairs(row) do
            for _,e in ipairs(col) do
                engine.entities:destroy_entity(e)
            end
            row[c] = {}
        end
    end

    coins = 0

    stage_list[current_stage]()
end
