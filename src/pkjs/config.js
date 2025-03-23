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
        "label": "Background Color"
      },
      {
        "type": "color",
        "messageKey": "timeColor",
        "defaultValue": "0xFFFFFF",
        "label": "Time Color"
      },
      {
        "type": "color",
        "messageKey": "dateColor",
        "defaultValue": "0xAAAAAA",
        "label": "Date Color"
      },
      {
        "type": "color",
        "messageKey": "weekColor",
        "defaultValue": "0xAAAAAA",
        "label": "Week Color"
      },
      {
        "type": "color",
        "messageKey": "highlightColor",
        "defaultValue": "0xFFFFFF",
        "label": "Highlight Color",
        "capabilities": ["COLOR"]
      }
    ]
  },
  {
    "type": "section",
    "items": [
      {
        "type": "heading",
        "defaultValue": "Fonts"
      },
      {
        "type": "select",
        "messageKey": "timeFont",
        "defaultValue": 1,
        "label": "Time Font",
        "options": [
          { "label": "LECO", "value": 1 },
          { "label": "Bitham Bold", "value": 2 },
          { "label": "Bitham Light", "value": 3 }
        ]
      },
      {
        "type": "select",
        "messageKey": "flavor",
        "defaultValue": "grape",
        "label": "Favorite Flavor",
        "options": [
          { "label": "", "value": "" },
          { "label": "Berry", "value": "berry" },
          { "label": "Grape", "value": "grape" },
          { "label": "Banana", "value": "banana" }
        ]
      }
    ]
  },
  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];