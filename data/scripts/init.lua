local engine = require('engine')
local player = require('archetypes.player')
local wall = require('archetypes.wall')
local coin = require('archetypes.coin')
local spike = require('archetypes.spike')

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
    fps = 0,
    version = ''
}

function create_entity(archetype, r, c, ...)
    table.insert(board[r][c], archetype(r, c, ...))
end

function add_stuff_at(str, r)
    for i = 1, #str do
        local c = str:sub(i, i)

        if c == '#' then
            create_entity(wall, r, i)
        elseif c == '@' then
            create_entity(player, r, i)
        elseif c == 'O' then
            create_entity(coin, r, i)
        elseif c == 'X' then
            create_entity(spike, r, i)
        end
    end
end

function create_stage()
    local r = 15

    local function next(str)
        add_stuff_at(str, r)
        r = r - 1
        return next
    end

    return next
end

stages = {
    function()
        create_stage()
            '               '
            '               '
            '               '
            '               '
            '       #       '
            '  #O      O#   '
            '     #O #      '
            '       @    #  '
            '    #          '
            '         @  O# '
            '   # O  X#     '
            '     #         '
            '               '
            '               '
            '               '
    end
}

current_stage = 1
stages[1]()

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

    stages[current_stage]()
end
