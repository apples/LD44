local vdom = require('vdom')

local coin_counter = vdom.component()

function coin_counter:constructor(props)
    assert(props.value)
    self:super(props)
end

function coin_counter:render()
    return vdom.create_element(
        'panel',
        {
            width = '12px',
            height = '20px',
            texture = tostring(self.props.value % 10),
        }
    )
end

return coin_counter
