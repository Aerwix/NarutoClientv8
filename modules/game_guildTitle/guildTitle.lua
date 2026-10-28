
local titleFont = "verdana-11px-rounded"
local OPCODE = 134

function init()
  connect(Creature, {
    onAppear = requestData,
  })
  ProtocolGame.registerExtendedOpcode(OPCODE, handleData)
end

function terminate()
  disconnect(Creature, {
    onAppear = requestData,
  })
  ProtocolGame.unregisterExtendedOpcode(OPCODE)
end


function requestData(creature)
  if creature:isPlayer() then
    local tbl = {
      creature = creature:getId()
    }
    local protocolGame = g_game.getProtocolGame()
    if protocolGame then
      protocolGame:sendExtendedJSONOpcode(OPCODE, tbl)
    end
  end
end

function handleData(protocol, code, buffer)
  local json_status, json_data = pcall(function ()
    return json.decode(buffer)
  end)

  if not json_status then
    g_logger.error("JSON error: " .. json_data)
    return false
  end

  if not json_data then
    return false
  end

  local response = json_data.response
  if response == "SetGuildName" then
    updateTitle(json_data.creatureId, json_data.guildName)
  end
end

local guildColors = {}

function updateTitle(creatureId, guildName)
    local target = g_map.getCreatureById(creatureId)
    if target then
        local color
        if guildColors[guildName] then
            color = guildColors[guildName]
        else
            local red = math.random(0, 255)
            local green = math.random(0, 255)
            local blue = math.random(0, 255)
            color = string.format("#%02X%02X%02X", red, green, blue)
            guildColors[guildName] = color
        end

        target:setTitle(guildName, titleFont, color)
    end
end
