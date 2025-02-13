
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
      "defaultValue": "ff0000",
      "label": "Background",
      "allowGray": "true",
      },

      {
        "type": "color",
        "messageKey": "primaryColor",
        "defaultValue": "0000ff",
        "label": "Time",
      },

      {
        "type": "color",
        "messageKey": "secondaryColor",
        "defaultValue": "00ff00",
        "label": "Date and Day",
      }
    ]
  },

  {
    "type": "submit",
    "defaultValue": "Save Settings"
  }
];