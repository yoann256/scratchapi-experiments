#include <curl/curl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>

#define BASE_URL "https://api.scratch.mit.edu/accounts/checkusername/"
#define RANDOM_STRING_LENGTH 4 // Length of generated usernames

volatile int running = 1;  // Flag to control the loop
int valid_count = 0;  // Counter for valid usernames
int total_checked = 0; // Counter for total usernames checked
FILE *file = NULL;  // File pointer

// Buffer to store API response
struct Response {
    char *data;
    size_t size;
};

// Callback function to store API response
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
    res->data[res->size] = '\0'; // Null-terminate the string
    return total_size;
}

// Function to generate a random string
void generate_random_string(char *str, size_t length) {
    static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (size_t i = 0; i < length; i++) {
        str[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    str[length] = '\0'; // Null-terminate the string
}

// Function to get the current date and time for the filename
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
        total_checked++; // Increment total usernames checked

        // Check if the response contains "username exists"
        if (strstr(res.data, "\"msg\":\"username exists\"")) {
            printf("❌ TAKEN: %s\n", username);
            if (file) fprintf(file, "❌ %s\n", username);  // Write to file
        } 
        // Check if the response contains "username is valid"
        else if (strstr(res.data, "\"msg\":\"valid username\"")) {
            valid_count++;  // Count valid usernames
            printf("✅ VALID: %s (Total: %d)\n", username, valid_count);
            if (file) fprintf(file, "✅ %s\n", username);  // Write to file
        } 
        // If neither, mark it as unknown
        else {
            printf("❔ UNKNOWN: %s\n", username);
            if (file) fprintf(file, "❔ %s\n", username);  // Write to file
        }
        free(res.data);
    }
}

// Function to handle API requests
void *request_loop(void *arg) {
    while (running) {
        char random_string[RANDOM_STRING_LENGTH + 1];
        generate_random_string(random_string, RANDOM_STRING_LENGTH);
        check_username(random_string);
        usleep(450000); // Delay between requests (0.45s)
    }
    return NULL;
}

// Function to listen for Enter key to stop
void *input_listener(void *arg) {
    getchar();  // Wait for Enter key press
    running = 0;
    return NULL;
}

int main() {
    printf("I, yoann256, am not responsible if you get banned from Scratch/Scratch API or got limited use to Scratch/Scratch API.\nThis project is purely educational and is not affiliated in any way with Scratch or the Scratch Team.\n");
    
    srand(time(NULL));  // Seed the random number generator

    // Generate a timestamped filename
    char filename[64];
    get_timestamped_filename(filename, sizeof(filename));
    
    // Open file for writing
    file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error: Could not open file for writing.\n");
        return 1;
    }

    printf("Saving results to: %s\n", filename);
    printf("Press Enter to stop...\n");

    // Create threads for username checking and input listening
    pthread_t request_thread, input_thread;
    pthread_create(&request_thread, NULL, request_loop, NULL);
    pthread_create(&input_thread, NULL, input_listener, NULL);

    // Wait for both threads to finish
    pthread_join(request_thread, NULL);
    pthread_join(input_thread, NULL);

    // Calculate accuracy percentage
    double accuracy = (total_checked > 0) ? ((double)valid_count / total_checked) * 100 : 0.0;

    // Print results
    printf("\nResults saved to %s\n", filename);
    printf("Total Usernames Checked: %d\n", total_checked);
    printf("Valid Usernames Found: %d\n", valid_count);
    printf("Accuracy: %.2f%%\n", accuracy);

    // Close file before exiting
    if (file) {
        fprintf(file, "\nTotal Usernames Checked: %d\n", total_checked);
        fprintf(file, "Valid Usernames Found: %d\n", valid_count);
        fprintf(file, "Accuracy: %.2f%%\n", accuracy);
        fclose(file);
    }

    return 0;
}
