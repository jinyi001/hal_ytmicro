{{ytCopyright "crc_config.c"}}


#include "crc_config.h"


{{#each this.data}}
/*{{this.name}}*/
{{#isTruthy this.readonly}}const {{/isTruthy}}crc_user_config_t {{this.name}} = {
    .crcWidth={{this.crcWidth}},
    .readTranspose={{this.readTranspose}},
    .writeTranspose={{this.writeTranspose}},
    .complementChecksum={{this.complementChecksum}},
    .seed={{this.seed}}
};
{{/each}}

