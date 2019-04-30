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
            height = 28,
            color = '#FDF7ED',
            left = 27 * 4,
            top = 35 * 4,
            text = tostring(self.props.coins),
        }
    )
end

return coin_counter
