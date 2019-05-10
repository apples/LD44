local vdom = require('vdom')
local counter = require('gui.counter')

local stage_counter = vdom.component()

function stage_counter:constructor(props)
    assert(props.stage)
    self:super(props)
end

function stage_counter:render()
    return vdom.create_element(
        'panel',
        {
            halign='left',
            valign='top',
            left = 31 * 4,
            top = 36 * 4,
        },
        vdom.create_element(counter, { value = self.props.stage })
    )
end

return stage_counter
