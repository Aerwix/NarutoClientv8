-- Party Status para OTCv8
-- Muestra los miembros de la party (vida/chakra) en una MiniWindow y los dibuja
-- en el minimapa como cruces de colores (igual que la cruz del jugador).
-- Servidor: data/scripts/creaturescripts/party_status.lua (opcode extendido 50).

local PARTY_OPCODE = 50
local UPDATE_INTERVAL = 2000 -- ms entre peticiones al servidor (solo mientras estas en party)

-- Colores de las cruces; cada miembro conserva su color mientras siga en la party
-- El lider siempre usa LEADER_COLOR; el resto toma colores de MARKER_COLORS.
-- Si hay mas miembros que colores, se repiten en ciclo.
local LEADER_COLOR = '#ffd700'
local MARKER_COLORS = { '#00ff00', '#00c8ff', '#ff66cc', '#ff8800', '#b266ff', '#66ffcc', '#ff4444', '#ccff33', '#4d79ff' }

-- Tamano de la cruz en el minimapa (px); el contorno negro es OUTLINE px mas grande
local MEMBER_CROSS_SIZE = 12
local LEADER_CROSS_SIZE = 16
local OUTLINE = 3

partyButton = nil
partyWindow = nil

local membersPanel = nil
local emptyLabel = nil
local minimap = nil
local updateEvent = nil

local markers = {}       -- name -> widget de la cruz en el minimapa
local memberColors = {}  -- name -> indice en MARKER_COLORS
local lastMembers = {}   -- ultima lista recibida del servidor

local function isInParty()
  local player = g_game.getLocalPlayer()
  if not player then return false end
  local shield = player:getShield()
  return shield >= ShieldBlue and shield <= ShieldYellowNoSharedExp
end

local function getMemberColor(m)
  if m.isLeader then
    memberColors[m.name] = nil -- el lider no ocupa color de la paleta
    return LEADER_COLOR
  end

  local index = memberColors[m.name]
  if not index then
    local used, count = {}, 0
    for _, i in pairs(memberColors) do
      used[i] = true
      count = count + 1
    end
    -- primer color libre; si no queda ninguno, se repite en ciclo
    index = (count % #MARKER_COLORS) + 1
    for i = 1, #MARKER_COLORS do
      if not used[i] then
        index = i
        break
      end
    end
    memberColors[m.name] = index
  end
  return MARKER_COLORS[index]
end

-- ===================== Minimapa =====================

local function styleMarker(marker, color, isLeader)
  local size = isLeader and LEADER_CROSS_SIZE or MEMBER_CROSS_SIZE
  local outlineSize = size + OUTLINE
  marker:setSize({ width = outlineSize, height = outlineSize })
  marker:getChildById('outline'):setSize({ width = outlineSize, height = outlineSize })
  local cross = marker:getChildById('cross')
  cross:setSize({ width = size, height = size })
  cross:setImageColor(color)
end

local function destroyMarker(name)
  local marker = markers[name]
  if marker then
    marker:destroy()
    markers[name] = nil
  end
end

local function clearMarkers()
  for name in pairs(markers) do
    destroyMarker(name)
  end
end

local function placeMarker(marker, pos)
  -- getTileRect() no es valido si el piso es distinto al de la camara del minimapa,
  -- y el widget acabaria en la esquina: hay que ocultarlo.
  if minimap and pos.z == minimap:getCameraPosition().z then
    minimap:centerInPosition(marker, pos) -- reemplaza el anclaje anterior
    marker:show()
  else
    marker:hide()
  end
end

local function updateMarkers(members)
  if not minimap then return end

  local seen = {}
  for _, m in ipairs(members) do
    seen[m.name] = true
    local marker = markers[m.name]
    if not marker then
      marker = g_ui.createWidget('PartyMinimapMarker', minimap)
      markers[m.name] = marker
    end
    -- se recalcula siempre: el liderazgo puede cambiar de mano
    local color = getMemberColor(m)
    if marker.baseColor ~= color or marker.isLeader ~= m.isLeader then
      marker.baseColor = color
      marker.isLeader = m.isLeader
      styleMarker(marker, color, m.isLeader)
    end
    marker.pos = m.pos
    marker:setTooltip(string.format('%s (%d%%)', m.name, m.healthPercent))
    placeMarker(marker, m.pos)
  end

  for name in pairs(markers) do
    if not seen[name] then
      destroyMarker(name)
    end
  end
end

-- La camara del minimapa cambia (te mueves, cambias de piso o usas los botones de piso)
local function onMinimapCameraChange()
  for _, marker in pairs(markers) do
    if marker.pos then
      placeMarker(marker, marker.pos)
    end
  end
end

-- ===================== Ventana =====================

local function getFloorText(memberZ)
  local player = g_game.getLocalPlayer()
  if not player then return '' end
  local diff = player:getPosition().z - memberZ -- en Tibia, z menor = mas arriba
  if diff > 0 then
    return '+' .. diff
  elseif diff < 0 then
    return tostring(diff)
  end
  return ''
end

local function updateWindow(members)
  if not membersPanel then return end

  membersPanel:destroyChildren()
  emptyLabel:setVisible(#members == 0)

  for _, m in ipairs(members) do
    local widget = g_ui.createWidget('PartyMemberPanel', membersPanel)

    widget:getChildById('colorIcon'):setImageColor(getMemberColor(m))

    local nameLabel = widget:getChildById('nameLabel')
    nameLabel:setText(m.isLeader and (m.name .. ' [L]') or m.name)
    nameLabel:setColor(m.isLeader and '#ffd700' or '#ffffff')

    local floorLabel = widget:getChildById('floorLabel')
    local floorText = getFloorText(m.pos.z)
    floorLabel:setText(floorText)
    floorLabel:setTooltip(floorText ~= '' and tr('Diferencia de piso') or '')

    local healthBar = widget:getChildById('healthBar')
    healthBar:setPercent(m.healthPercent)
    healthBar:setText(m.healthPercent .. '%')
    healthBar:setTooltip(m.health .. ' / ' .. m.maxHealth)

    local manaBar = widget:getChildById('manaBar')
    manaBar:setPercent(m.manaPercent)
    manaBar:setText(m.manaPercent .. '%')
    manaBar:setTooltip(m.mana .. ' / ' .. m.maxMana)
  end
end

-- ===================== Red =====================

local function percent(value, max)
  if max <= 0 then return 0 end
  return math.max(0, math.min(100, math.floor(value * 100 / max)))
end

local function parseMembers(buffer)
  local members = {}
  if not buffer or buffer == '' then
    return members
  end

  local localPlayer = g_game.getLocalPlayer()
  local localName = localPlayer and localPlayer:getName() or ''

  for entry in string.gmatch(buffer, '[^;]+') do
    local f = {}
    for field in string.gmatch(entry, '[^,]+') do
      f[#f + 1] = field
    end

    local name = f[1]
    if name and name ~= localName then
      local health, maxHealth = tonumber(f[2]) or 0, tonumber(f[3]) or 1
      local mana, maxMana = tonumber(f[4]) or 0, tonumber(f[5]) or 1
      members[#members + 1] = {
        name = name,
        health = health, maxHealth = maxHealth,
        mana = mana, maxMana = maxMana,
        healthPercent = percent(health, maxHealth),
        manaPercent = percent(mana, maxMana),
        pos = { x = tonumber(f[6]) or 0, y = tonumber(f[7]) or 0, z = tonumber(f[8]) or 7 },
        isLeader = f[9] == '1',
      }
    end
  end
  return members
end

local function applyMembers(members)
  lastMembers = members

  -- liberar colores de quien ya no esta
  local present = {}
  for _, m in ipairs(members) do present[m.name] = true end
  for name in pairs(memberColors) do
    if not present[name] then memberColors[name] = nil end
  end

  updateMarkers(members)
  updateWindow(members)
end

local function onPartyData(protocol, opcode, buffer)
  applyMembers(parseMembers(buffer))
end

local function stopUpdates()
  if updateEvent then
    removeEvent(updateEvent)
    updateEvent = nil
  end
end

local function requestPartyUpdate()
  updateEvent = nil
  if not g_game.isOnline() then return end

  if isInParty() then
    local protocol = g_game.getProtocolGame()
    if protocol then
      protocol:sendExtendedOpcode(PARTY_OPCODE, '')
    end
  elseif #lastMembers > 0 then
    applyMembers({})
  end

  updateEvent = scheduleEvent(requestPartyUpdate, UPDATE_INTERVAL)
end

local function restartUpdates()
  stopUpdates()
  requestPartyUpdate()
end

-- Al entrar/salir de party: actualizar al momento, sin esperar al siguiente ciclo
local function onShieldChange(creature, shield)
  if creature:isLocalPlayer() then
    restartUpdates()
  end
end

local function onPositionChange(creature, newPos, oldPos)
  if newPos and oldPos and newPos.z ~= oldPos.z and #lastMembers > 0 then
    updateWindow(lastMembers) -- refresca la diferencia de piso
  end
end

local function online()
  restartUpdates()
end

local function offline()
  stopUpdates()
  memberColors = {}
  applyMembers({})
end

-- ===================== UI =====================

function toggle()
  if partyButton:isOn() then
    partyWindow:close()
  else
    partyWindow:open()
  end
end

function onMiniWindowOpen()
  if partyButton then
    partyButton:setOn(true)
  end
end

function onMiniWindowClose()
  if partyButton then
    partyButton:setOn(false)
  end
end

-- ===================== Ciclo de vida =====================

function init()
  minimap = modules.game_minimap and modules.game_minimap.minimapWidget or nil
  if not minimap then
    print('[PartyStatus] game_minimap no disponible: no se dibujaran cruces')
  end

  partyButton = modules.client_topmenu.addRightGameToggleButton('partyStatusButton', tr('Party'), '/images/topbuttons/viplist', toggle, false, 7)
  partyButton:setOn(false)

  partyWindow = g_ui.loadUI('party_status', modules.game_interface.getRightPanel())
  membersPanel = partyWindow:recursiveGetChildById('membersPanel')
  emptyLabel = partyWindow:recursiveGetChildById('emptyLabel')
  partyWindow:setup()
  partyButton:setOn(partyWindow:isVisible())

  ProtocolGame.registerExtendedOpcode(PARTY_OPCODE, onPartyData)
  connect(g_game, { onGameStart = online, onGameEnd = offline })
  connect(LocalPlayer, { onShieldChange = onShieldChange, onPositionChange = onPositionChange })
  if minimap then
    connect(minimap, { onCameraPositionChange = onMinimapCameraChange })
  end

  if g_game.isOnline() then
    online()
  end
end

function terminate()
  stopUpdates()
  disconnect(g_game, { onGameStart = online, onGameEnd = offline })
  disconnect(LocalPlayer, { onShieldChange = onShieldChange, onPositionChange = onPositionChange })
  if minimap then
    disconnect(minimap, { onCameraPositionChange = onMinimapCameraChange })
  end
  ProtocolGame.unregisterExtendedOpcode(PARTY_OPCODE)

  clearMarkers()
  memberColors = {}
  lastMembers = {}

  if partyWindow then
    partyWindow:destroy()
    partyWindow = nil
  end
  if partyButton then
    partyButton:destroy()
    partyButton = nil
  end
  minimap = nil
end
