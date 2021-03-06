local vdom = require('vdom')
local counter = require('gui.counter')

local coin_counter = vdom.component()

function coin_counter:constructor(props)
    assert(props.coins)
    self:super(props)
end

function coin_counter:render()
    return vdom.create_element(
        'panel',
        {
            halign='left',
            valign='top',
            left = 31 * 4,
            top = 42 * 4,
        },
        vdom.create_element(counter, { value = self.props.coins })
    )
end

return coin_counter
