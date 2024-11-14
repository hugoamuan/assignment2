#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NAME_LEN 100
#define MAX_LINE_LEN 200
#define MAX_MONTH_LEN 4
#define MIN_YEAR 1950
#define MAX_YEAR 2010
#define MAX_TOEFL 120
#define MIN_TOEFL 0

typedef struct {
    char firstName[MAX_NAME_LEN];
    char lastName[MAX_NAME_LEN];
    char month[MAX_MONTH_LEN];
    int day;
    int year;
    float gpa;
    char status;
    int toefl;  // -1 for domestic students
} Student;

// Function to convert month string to number for comparison
int monthToNum(const char *month) {
    const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                           "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    for (int i = 0; i < 12; i++) {
        if (strcasecmp(month, months[i]) == 0) return i + 1;
    }
    return -1;
}

// Validate month string
int isValidMonth(const char *month) {
    return monthToNum(month) != -1;
}

// Validate date components
int isValidDate(const char *month, int day, int year) {
    if (!isValidMonth(month)) return 0;
    if (year < MIN_YEAR || year > MAX_YEAR) return 0;

    // Simplified day validation (could be more precise with month lengths)
    if (day < 1 || day > 31) return 0;

    return 1;
}

// New version (correct):
int compareStudents(const Student *a, const Student *b) {
    // 1. Year of birth
    if (a->year != b->year) return a->year - b->year;

    // 2. Month of birth
    int monthDiff = monthToNum(a->month) - monthToNum(b->month);
    if (monthDiff != 0) return monthDiff;

    // 3. Day of birth
    if (a->day != b->day) return a->day - b->day;

    // 4. Last name (case insensitive)
    int lastNameCmp = strcasecmp(a->lastName, b->lastName);
    if (lastNameCmp != 0) return lastNameCmp;

    // 5. First name (case insensitive)
    int firstNameCmp = strcasecmp(a->firstName, b->firstName);
    if (firstNameCmp != 0) return firstNameCmp;

    // 6. GPA (changed to sort in descending order)
    if (a->gpa != b->gpa) return (a->gpa < b->gpa) ? 1 : -1;

    // 7. TOEFL (descending order, domestic students take precedence if no TOEFL)
    if (a->toefl != b->toefl) {
        if (a->toefl == -1) return -1;  // a is domestic
        if (b->toefl == -1) return 1;   // b is domestic
        return (a->toefl < b->toefl) ? 1 : -1;  // Changed to sort in descending order
    }

    // 8. Domestic > International
    return (b->status == 'D') - (a->status == 'D');
}


// Merge function for merge sort
void merge(Student arr[], int left, int mid, int right) {
    int i, j, k;
    int n1 = mid - left + 1;
    int n2 = right - mid;

    Student *L = malloc(n1 * sizeof(Student));
    Student *R = malloc(n2 * sizeof(Student));

    if (!L || !R) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    for (i = 0; i < n1; i++)
        L[i] = arr[left + i];
    for (j = 0; j < n2; j++)
        R[j] = arr[mid + 1 + j];

    i = 0;
    j = 0;
    k = left;

    while (i < n1 && j < n2) {
        if (compareStudents(&L[i], &R[j]) <= 0)
            arr[k++] = L[i++];
        else
            arr[k++] = R[j++];
    }

    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];

    free(L);
    free(R);
}

// Merge sort implementation
void mergeSort(Student arr[], int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

int parseLine(char *line, Student *student) {
    char monthStr[MAX_MONTH_LEN];
    int day, year;
    int toeflScore = -1;  // Default TOEFL for domestic students
    int result;

    // Attempt to parse as international student (with optional TOEFL score)
    result = sscanf(line, "%s %s %3s-%d-%d %f %c %d",
                    student->firstName,
                    student->lastName,
                    monthStr,
                    &day,
                    &year,
                    &student->gpa,
                    &student->status,
                    &toeflScore);

    // Handle both domestic and international formats
    if (result == 8 && student->status == 'I') {
        if (toeflScore < MIN_TOEFL || toeflScore > MAX_TOEFL) {
            fprintf(stderr, "Error: Invalid TOEFL score for international student.\n");
            return 0;
        }
        student->toefl = toeflScore;
    } else if (result == 7 && student->status == 'D') {
        student->toefl = -1;  // Domestic student, no TOEFL score
    } else if (result == 7 && student->status == 'I') {
        student->toefl = 0;  // International without specified TOEFL, default to 0
    } else {
        fprintf(stderr, "Error: Invalid input format in line: %s\n", line);
        return 0;
    }

    // Validate the parsed month, day, and year
    if (!isValidDate(monthStr, day, year)) {
        fprintf(stderr, "Error: Invalid date in line: %s\n", line);
        return 0;
    }

    // Copy validated date information to the student structure
    strcpy(student->month, monthStr);
    student->day = day;
    student->year = year;

    // Validate GPA range
    if (student->gpa < 0.0 || student->gpa > 4.301) {
        fprintf(stderr, "Error: Invalid GPA in line: %s\n", line);
        return 0;
    }

    return 1;  // Successfully parsed and validated
}


// In main(), update the error handling:
int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <input file> <output file> <option>\n", argv[0]);
        return 1;
    }

    const char *inputFile = argv[1];
    const char *outputFile = argv[2];
    int option = atoi(argv[3]);

    if (option < 1 || option > 3) {
        fprintf(stderr, "Error: Invalid option. Must be 1, 2, or 3.\n");
        return 1;
    }

    FILE *in = fopen(inputFile, "r");
    if (!in) {
        fprintf(stderr, "Error: Could not open input file.\n");
        return 1;
    }

    FILE *out = fopen(outputFile, "w");
    if (!out) {
        fprintf(stderr, "Error: Could not open output file.\n");
        fclose(in);
        return 1;
    }

    Student *students = NULL;
    size_t count = 0, capacity = 100;
    students = malloc(capacity * sizeof(Student));
    if (!students) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(in);
        fclose(out);
        return 1;
    }

    char line[MAX_LINE_LEN];
    while (fgets(line, sizeof(line), in)) {
        Student student;
        if (!parseLine(line, &student)) {
            fprintf(out, "Error: Invalid input format.\n");
            free(students);
            fclose(in);
            fclose(out);
            return 1;
        }

        // Filter based on option
        if ((option == 1 && student.status != 'D') ||
            (option == 2 && student.status != 'I')) {
            continue;
        }

        // Dynamically resize if needed
        if (count == capacity) {
            capacity *= 2;
            Student *temp = realloc(students, capacity * sizeof(Student));
            if (!temp) {
                fprintf(stderr, "Memory allocation failed\n");
                free(students);
                fclose(in);
                fclose(out);
                return 1;
            }
            students = temp;
        }

        students[count++] = student;
    }

    fclose(in);

    // Sort the student array
    mergeSort(students, 0, count - 1);

    // Write sorted output to the file
    for (size_t i = 0; i < count; i++) {
        Student *s = &students[i];
        fprintf(out, "%s %s %s-%d-%d %.3f %c",
                s->firstName, s->lastName, s->month, s->day, s->year, s->gpa, s->status);
        if (s->status == 'I') {
            fprintf(out, " %d", s->toefl);
        }
        fprintf(out, "\n");
    }

    free(students);
    fclose(out);
    return 0;
}


