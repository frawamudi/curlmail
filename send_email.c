#include "src/email.h"
#include <stdio.h>

int main(void)
{
    int rc = send_email("Header/Source split test",
                        "This is a test email with attachments.\r\n");
    if (rc == 0)
        printf(" Email sent successfully\n");
    else
        printf(" Failed to send email (code %d)\n", rc);

    return rc;
}
