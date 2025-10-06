#ifndef EMAIL_H
#define EMAIL_H

#include <curl/curl.h>

/**
 * Load environment variables from a .env file.
 * Each line should be in the format KEY=VALUE.
 * Comments (#) and empty lines are ignored.
 *
 * @param filename path to the .env file
 * @return 0 on success, -1 on failure
 */
int load_env_file(const char *filename);

/**
 * Send an email using Gmail SMTP with optional CC, BCC, and attachments.
 * Reads SMTP_USER, SMTP_PASS, SMTP_TO, SMTP_CC, SMTP_BCC, SMTP_ATTACH
 * from the environment.
 *
 * @param subject Subject line for the email
 * @param body    Plain-text message body
 * @return 0 on success, nonzero on failure
 */
int send_email(const char *subject, const char *body);

#endif /* EMAIL_H */
