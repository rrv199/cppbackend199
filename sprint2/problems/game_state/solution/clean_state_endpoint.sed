/if (target == "api\/v1\/game\/state") {/,/^        }$/ {
    /auto response = HandleGameState(token);/ {
        n
        /auto response = HandleGameState(token);/ d
        /send(std::move(response));/ {
            n
            /return;$/ {
                n
                /} else {/ {
                    n
                    /auto response = MakeJsonResponse/,/^                send/ {
                        /^                send/ {
                            n
                            /return;$/ {
                                n
                                /^                }$/ d
                            }
                        }
                    }
                }
            }
        }
    }
}
