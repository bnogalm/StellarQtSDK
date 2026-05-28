#ifndef INVALIDSEP45CHALLENGEEXCEPTION_H
#define INVALIDSEP45CHALLENGEEXCEPTION_H

#include <stdexcept>
#include <string>

namespace qstellar { namespace exception {

/**
 * Thrown by `Sep45Challenge::verifyChallenge` when a challenge fails any
 * of the static or simulation checks. Carries a human-readable reason —
 * NOT the underlying XDR / network detail — so it is safe to log.
 */
class InvalidSep45ChallengeException : public std::runtime_error
{
public:
    explicit InvalidSep45ChallengeException(const std::string& reason)
        : std::runtime_error(reason) {}
};

}} // namespace qstellar::exception

#endif // INVALIDSEP45CHALLENGEEXCEPTION_H
