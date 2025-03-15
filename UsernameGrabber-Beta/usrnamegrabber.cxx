#include <curl/curl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include "libs/parson.h"  // Using Parson for JSON handling
#include "libs/parson.c"

#define BASE_URL "https://api.scratch.mit.edu/accounts/checkusername/"

volatile int running = 1;
int valid_count = 0;
int total_checked = 0;
int delay_old = 450000;  // Default delay (0.45s)
int delay = 500;
int username_length = 4;  // Default username length
FILE *file = NULL;

// Struct for API response buffer
struct Response {
    char *data;
    size_t size;
};

// Callback function for cURL to write API response
size_t writefunc(void *ptr, size_t size, size_t nmemb, struct Response *res) {
    size_t total_size = size * nmemb;
    char *new_data = (char *)realloc(res->data, res->size + total_size + 1);
    
    if (new_data == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 0;
    }
    
    res->data = new_data;
    memcpy(res->data + res->size, ptr, total_size);
    res->size += total_size;
    res->data[res->size] = '\0';  // Null-terminate
    return total_size;
}

// Function to generate a random string
void generate_random_string(char *str, size_t length) {
    static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (size_t i = 0; i < length; i++) {
        str[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    str[length] = '\0';  // Null-terminate
}

// JSON Configuration Handling
void load_or_create_config() {
    JSON_Value *root_value = json_parse_file("config.json");

    if (root_value == NULL) {
        printf("Config file not found, creating one with default values...\n");

        root_value = json_value_init_object();
        JSON_Object *root_object = json_value_get_object(root_value);
        json_object_set_number(root_object, "delay", 450);  // Save in seconds
        json_object_set_number(root_object, "username_length", 4);
        json_serialize_to_file(root_value, "config.json");

        printf("Default config file created.\n");
    } else {
        JSON_Object *root_object = json_value_get_object(root_value);
        
        // Check if keys exist before using them
        if (json_object_has_value(root_object, "delay")) {
            delay = (int)(json_object_get_number(root_object, "delay"));  // Convert to ms
        } else {
            printf("Warning: 'delay' not found in config, using default.\n");
        }

        if (json_object_has_value(root_object, "username_length")) {
            username_length = (int)json_object_get_number(root_object, "username_length");
        } else {
            printf("Warning: 'username_length' not found in config, using default.\n");
        }

        printf("Loaded config: Delay = %dms, Username Length = %d\n", delay, username_length);
    }

    json_value_free(root_value);
}

// Function to generate a timestamped filename
void get_timestamped_filename(char *filename, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    snprintf(filename, size, "foundusernames_%04d-%02d-%02d_%02d-%02d-%02d.txt",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
             t->tm_hour, t->tm_min, t->tm_sec);
}

// Function to check if the username exists
void check_username(const char *username) {
    char url[256];
    snprintf(url, sizeof(url), "%s%s", BASE_URL, username);
    
    CURL *curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize cURL\n");
        return;
    }

    struct Response res = {NULL, 0};

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &res);
    curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res.data) {
        total_checked++;

        if (strstr(res.data, "\"msg\":\"username exists\"")) {
            printf("❌ TAKEN: %s\n", username);
            if (file) fprintf(file, "❌ %s\n", username);
        } else if (strstr(res.data, "\"msg\":\"valid username\"")) {
            valid_count++;
            printf("✅ VALID: %s (Total: %d)\n", username, valid_count);
            if (file) fprintf(file, "✅ %s\n", username);
        } else {
            printf("❔ UNKNOWN: %s\n", username);
            if (file) fprintf(file, "❔ %s\n", username);
        }
        free(res.data);
    }
}

// Function to handle API requests
void *request_loop(void *arg) {
    while (running) {
        if (!running) break;  // Double-check before generating a username
        
        char random_string[username_length + 1];
        generate_random_string(random_string, username_length);
        check_username(random_string);

        if (!running) break;  // Another check before sleeping
        usleep(delay * 1000);
    }
    printf("Stopped.\n");
    return NULL;
}



// Function to listen for Enter key to stop
void *input_listener(void *arg) {
    getchar();
    printf("Stopping...\n");
    running = 0;
    return NULL;
}

int main() {
    printf("I, yoann256, am not responsible if you get banned from Scratch/Scratch API or got limited use to Scratch/Scratch API.\nThis project is purely educational and is not affiliated in any way with Scratch or the Scratch Team.\n");

    srand(time(NULL));  // Seed the random number generator

    // **Mode Selection**
    int mode;
    printf("Select Mode:\n");
    printf("1. Normal Mode (0.5s delay)\n");
    printf("2. Fast Mode (0.3s delay)\n");
    printf("3. Ultra Mode (0.2s delay) [WARNING: Might get rate-limited!]\n");
    printf("4. Custom Mode\n");
    printf("5. Load from JSON\n");
    printf("Enter your choice: ");
    scanf("%d", &mode);

    // **Clear input buffer**
    while (getchar() != '\n');  

    switch (mode) {
        case 1: delay = 500; break;
        case 2: delay = 300; break;
        case 3: 
            printf("WARNING: Ultra Mode may cause rate-limiting!\n");
            delay = 200;
            break;
        case 4: 
            printf("Enter custom delay (in ms): ");
            scanf("%d", &delay);
            while (getchar() != '\n');  // **Clear buffer again**
            break;
        case 5: 
            load_or_create_config(); 
            break;
        default: 
            printf("Invalid choice! Defaulting to Normal Mode.\n");
            delay = 500;
    }

    // **File Handling**
    char filename[64];
    get_timestamped_filename(filename, sizeof(filename));

    file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error: Could not open file for writing.\n");
        return 1;
    }

    printf("Saving results to: %s\n", filename);
    printf("Press Enter to stop...\n");

    // **Start Threads**
    pthread_t request_thread, input_thread;
    pthread_create(&request_thread, NULL, request_loop, NULL);
    pthread_create(&input_thread, NULL, input_listener, NULL);

    // **Wait for Threads to Finish**
    pthread_join(request_thread, NULL);
    pthread_join(input_thread, NULL);

    // **Final Statistics**
    double accuracy = (total_checked > 0) ? ((double)valid_count / total_checked) * 100 : 0.0;
    printf("\nResults saved to %s\n", filename);
    printf("Total Usernames Checked: %d\n", total_checked);
    printf("Valid Usernames Found: %d\n", valid_count);
    printf("Accuracy: %.2f%%\n", accuracy);

    // **Write Stats to File**
    if (file) {
        fprintf(file, "\nTotal Usernames Checked: %d\n", total_checked);
        fprintf(file, "Valid Usernames Found: %d\n", valid_count);
        fprintf(file, "Accuracy: %.2f%%\n", accuracy);
        fclose(file);
    }

    return 0;
}
