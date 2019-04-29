local player = require('archetypes.player')
local wall = require('archetypes.wall')
local coin = require('archetypes.coin')
local spike = require('archetypes.spike')
local exit = require('archetypes.exit')

local function create_entity(archetype, r, c, ...)
    table.insert(board[r][c], archetype(r, c, ...))
end

local function add_stuff_at(str, r)
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
        elseif c == '>' then
            create_entity(exit, r, i)
        end
    end
end

local function create_stage(str)
    local r = 15

    local function next(str)
        add_stuff_at(str, r)
        r = r - 1
        return next
    end

    return next(str)
end

local stages = {}

function stages.teamwork()
    create_stage
        '               '
        '               '
        '               '
        '               '
        '       X       '
        '  XO      OX   '
        '     #O #      '
        '       @    #  '
        '    #          '
        '         @  O# '
        '   X O  X#     '
        '     #         '
        '          >    '
        '               '
        '               '
end

function stages.winner()
    create_stage
        ' # #  #  # #   '
        ' # # # # # #   '
        '  #  # # # #   '
        '  #   #   #    '
        '               '
        '#   # # #  # # '
        '#   # # ## # # '
        '# # # # # ##   '
        ' # #  # #  # # '
        '               '
        '###############'
        '##OOOOOOOOOOOO#'
        '#OOOOOO@OOOOOO#'
        '#OOOOOOOOOOOO##'
        '###############'
end

return stages
