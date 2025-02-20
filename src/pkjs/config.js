module.exports = [
  {
    "type": "heading",
    "defaultValue": "Blotch"
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Colors"
      },

      {
        "type": "color",
        "messageKey": "backgroundColor",
        "defaultValue": "0x000055",
        "label": "Background Color",
      },

      {
        "type": "color",
        "messageKey": "timeColor",
        "defaultValue": "0xFFFFFF",
        "label": "Time Color",
        "allowGray": true,
      },

      {
        "type": "color",
        "messageKey": "dateColor",
        "defaultValue": "0xAAAAAA",
        "label": "Date Color",
        "allowGray": true,
      },

      {
        "type": "color",
        "messageKey": "weekColor",
        "defaultValue": "0xAAAAAA",
        "label": "Week Color",
        "allowGray": true,
      },

      {
        "type": "color",
        "messageKey": "highlightColor",
        "defaultValue": "0xFFFFFF",
        "label": "Highlight Color",
        "allowGray": true,
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];