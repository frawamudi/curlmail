#include "email.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Documentation:
// https://curl.se/libcurl/c/libcurl-tutorial.html

/* ========== .env loader ========== */
int load_env_file(const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file) return -1;

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = 0;
        if (line[0] == '#' || line[0] == '\0')
            continue;

        char *eq = strchr(line, '=');
        if (!eq) continue;

        *eq = '\0';
        char *key = line;
        char *val = eq + 1;

        setenv(key, val, 1);
    }

    fclose(file);
    return 0;
}

/* ========== helper to add recipients ========== */
static struct curl_slist *add_recipients(struct curl_slist *list, const char *env)
{
    if (!env) return list;

    char *copy = strdup(env);
    if (!copy) return list;

    char *token = strtok(copy, ",");
    while (token) {
        while (*token == ' ') token++; /* trim leading spaces */
        if (*token != '\0')
            list = curl_slist_append(list, token);
        token = strtok(NULL, ",");
    }

    free(copy);
    return list;
}

/* ========== send_email with attachments ========== */
int send_email(const char *subject, const char *body)
{
    load_env_file(".env");

    const char *user    = getenv("SMTP_USER");
    const char *pass    = getenv("SMTP_PASS");
    const char *to      = getenv("SMTP_TO");
    const char *cc      = getenv("SMTP_CC");
    const char *bcc     = getenv("SMTP_BCC");
    const char *attachs = getenv("SMTP_ATTACH");

    if (!user || !pass || !to) {
        fprintf(stderr, "Missing SMTP_USER, SMTP_PASS or SMTP_TO in environment\n");
        return 1;
    }

    CURL *curl = NULL;
    CURLcode res = CURLE_OK;
    struct curl_slist *recipients = NULL;

    curl = curl_easy_init();
    if (!curl) return 2;

    /* ========== Build MIME message ========== */
    curl_mime *mime;
    curl_mimepart *part;

    mime = curl_mime_init(curl);

    /* Add text body */
    part = curl_mime_addpart(mime);
    curl_mime_data(part, body, CURL_ZERO_TERMINATED);
    curl_mime_type(part, "text/plain; charset=UTF-8");

    /* Add attachments if defined */
    if (attachs) {
        char *copy = strdup(attachs);
        char *token = strtok(copy, ",");
        while (token) {
            while (*token == ' ') token++; /* trim */
            if (*token != '\0') {
                part = curl_mime_addpart(mime);
                curl_mime_filedata(part, token);
            }
            token = strtok(NULL, ",");
        }
        free(copy);
    }

    /* Recipients */
    recipients = add_recipients(recipients, to);
    recipients = add_recipients(recipients, cc);
    recipients = add_recipients(recipients, bcc);

    /* SMTP setup */
    curl_easy_setopt(curl, CURLOPT_URL, "smtps://smtp.gmail.com:465");
    curl_easy_setopt(curl, CURLOPT_USERNAME, user);
    curl_easy_setopt(curl, CURLOPT_PASSWORD, pass);
    curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
    curl_easy_setopt(curl, CURLOPT_MAIL_FROM, user);
    curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

    /* Headers (From, To, Cc, Subject) */
    struct curl_slist *headers = NULL;
    char from_hdr[256], to_hdr[512], cc_hdr[512], subj_hdr[256];
    snprintf(from_hdr, sizeof(from_hdr), "From: %s", user);
    snprintf(to_hdr,   sizeof(to_hdr),   "To: %s", to);
    snprintf(subj_hdr, sizeof(subj_hdr), "Subject: %s", subject);

    headers = curl_slist_append(headers, from_hdr);
    headers = curl_slist_append(headers, to_hdr);
    if (cc) {
        snprintf(cc_hdr, sizeof(cc_hdr), "Cc: %s", cc);
        headers = curl_slist_append(headers, cc_hdr);
    }
    headers = curl_slist_append(headers, subj_hdr);

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    /* Attach MIME body */
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    /* Send */
    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));

    /* Cleanup */
    curl_slist_free_all(recipients);
    curl_slist_free_all(headers);
    curl_mime_free(mime);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK) ? 0 : 5;
}
