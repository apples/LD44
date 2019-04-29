local vdom = require('vdom')
local fps_counter = require('gui.fps_counter')
local coin_counter = require('gui.coin_counter')
local stage_counter = require('gui.stage_counter')

local gameplay_gui = vdom.component()

function gameplay_gui:constructor(props)
    assert(props.initial_state)
    self:super(props)

    self.state = props.initial_state
end

function gameplay_gui:render()
    return vdom.create_element('widget', { width = '100%', height = '100%' },
        vdom.create_element('panel',
            {
                width = '25%',
                height = '100%',
                texture = 'panel',
            },
            vdom.create_element(stage_counter, { stage = self.state.stage }),
            vdom.create_element(coin_counter, { coins = self.state.coins })
        )
    )
end

return gameplay_gui
