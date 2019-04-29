local vdom = require('vdom')

local coin_counter = vdom.component()

function coin_counter:constructor(props)
    assert(props.coins)
    self:super(props)
end

function coin_counter:render()
    return vdom.create_element(
        'label',
        {
            halign='left',
            valign='top',
            height = 24,
            color = '#f0f',
            text = 'Coins: ' .. self.props.coins,
        }
    )
end

return coin_counter
