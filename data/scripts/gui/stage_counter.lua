local vdom = require('vdom')

local stage_counter = vdom.component()

function stage_counter:constructor(props)
    assert(props.stage)
    self:super(props)
end

function stage_counter:render()
    return vdom.create_element(
        'label',
        {
            halign='left',
            valign='top',
            height = 28,
            color = '#417089',
            left = 27 * 4,
            top = 28 * 4,
            text = tostring(self.props.stage),
        }
    )
end

return stage_counter
