#include <QFile>
#include <QList>
#include <QTextStream>
#include <QRegExp>
#include <QDebug>

#include <iostream>

// Matching map
const QChar letters[] = {'s', 't', 'n', 'm', 'r', 'l', 'd', 'k', 'f', 'p'};

QStringList find_words(const QStringList& dictionary, const QString& sequence) {
    QStringList regex_builder;

    // Build the regex string
    QString joker = "[aeiou]*";
    regex_builder << "^" << joker;
    for (const auto& c : sequence) {
        regex_builder << letters[c.digitValue()] << joker;
    }
    regex_builder << "$";

    // Compile it
    QRegExp regex(regex_builder.join(""));

    // Process words
    QStringList matches;
    for (const auto& word: dictionary) {
        if (regex.exactMatch(word) && word.size() >= 2) {
            matches << word;
        }
    }

    return matches;
}

struct CombinationItem {
    QStringList words;
    QList<CombinationItem> children;
};

// FIXME: strangely, OpenMP doesn't speed things up...
//        Thread congestion? Prepartition?
QList<CombinationItem> find_combinations(const QStringList& dictionary, const QString& sequence) {
    QList<CombinationItem> combinations;

    #pragma omp parallel for
    for (int len = 1; len <= sequence.length(); len++) {
        CombinationItem combination;

        // Extract the current subsequence, and match with words
        // TODO: change to leftRef when 5.4 is released
        QString current = sequence.left(len);
        QStringList current_words = find_words(dictionary, current);
        if (current_words.size() > 0) {
            combination.words = current_words;

            // Extract the remainder, and to the same
            int remainder_length = sequence.length() - len;
            if (remainder_length > 0) {
                // TODO: change to rightRef when 5.4 is released
                QString remainder = sequence.right(sequence.length() - len);
                QList<CombinationItem> remainder_combinations = find_combinations(dictionary, remainder);
                if (remainder_combinations.size() > 0) {
                    // Found valid child combinations
                    combination.children = remainder_combinations;
                    #pragma omp critical
                    combinations << combination;
                }
            } else {
                // No remainder, we've reached the end
                #pragma omp critical
                combinations << combination;
            }
        }
    }

    return combinations;
}

QList<QList<QStringList>> flatten(QList<CombinationItem> combinations) {
    QList<QList<QStringList>> flattened;

    // List all possible combinations (each having a set of matches for a different subsequence)
    for (const auto &combination: combinations) {
        // Check whether we still have a list of combinations for the rest of the sequence
        if (combination.children.length() > 0) {
            QList<QList<QStringList>> remainder_flattened = flatten(combination.children);
            for (auto &sublist: remainder_flattened)
                sublist.prepend(combination.words);
            flattened << remainder_flattened;
        }
        // If not, we reached the end of the sequence
        else {
            flattened << QList<QStringList>{combination.words};
        }
    }

    return flattened;
}

// Comparator which compares the length of elements in a string or container
//
// Used to sort a list from shortest to longest word, and to sort the
// partitioning from the shortest to the longest
template<typename T>
struct LengthComparator {
    bool operator()(T& a, T& b) const {
        return a.length() <= b.length();
    }
};

void display_partitioning(QList<QStringList>& partitioning) {
    // NOTE: each partitioning is a fixed constellation (eg. 123456 -> 12 345 6)
    // with a varying set of possibilities for each set
    int partitions = partitioning.size();   // can be expensive on non-const arg

    // Determine the maximal amount of options (determines the rowcount) and
    // character length (determines the column width)
    int rowcount = 0;
    QList<unsigned int> colwidths;
    colwidths.reserve(partitions);
    for (auto& options: partitioning) {
        rowcount = qMax(rowcount, options.size());
        int colwidth = 0;
        for (const auto word: options)
            colwidth = qMax(colwidth, word.length());
        colwidths.append(colwidth);

        // Also sort each list of words
        qSort(options.begin(), options.end(), LengthComparator<QString>());
    }

    QTextStream s(stdout);
    for (int row = 0; row < rowcount; ++row) {
        for (int column = 0; column < partitions; ++column) {
            s.setFieldWidth(colwidths[column]+3);
            s << (row < partitioning[column].size() ? partitioning[column][row] : " ");
        }
        s << "\n";
    }
    s << "\n\n";
    s.flush();
}


int main(int argc, char *argv[])
{
    // Prompt for word
    QString sequence;
    if (argc == 1) {
        std::cout << "Number sequence to remember: " << std::flush;
        QTextStream s(stdin);
        sequence = s.readLine();
    } else {
        sequence = argv[1];
    }

    // Open dictionary
    qDebug() << "Opening dictionary";
    QFile dict("dutch.dat");
    if (!dict.open(QIODevice::ReadOnly | QIODevice::Text)) {
        std::cerr << "Could not open file" << std::endl;
        return -1;
    }

    // Read words
    qDebug() << "Reading words";
    QStringList words;
    QTextStream in(&dict);
    while (!in.atEnd()) {
        QString line = in.readLine();
        words.append(line);
    }
    dict.close();

    // Find combinations
    qDebug() << "Finding combinations";
    QList<CombinationItem> combinations = find_combinations(words, sequence);

    // Flatten
    qDebug() << "Flattening";
    QList<QList<QStringList>> flattened = flatten(combinations);

    // Display
    qSort(flattened.end(), flattened.begin(), LengthComparator<QList<QStringList>>());
    for (auto& partitioning: flattened) {
        display_partitioning(partitioning);
    }

    return 0;
}
