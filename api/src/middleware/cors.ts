import * as _ from "lodash";

enum corsType {
    express = 0,
    socket = 1,
}

const corsOption = (type: corsType) => {
    if (process.env.NODE_ENV === "production") {

        let origin: Array<string> = ["https://smartpv.strubel.io", "http://localhost:4200", "http://192.168.178.200:9080"];
        if (type === corsType.socket) {
            const optionsProd: any = {
                cors: {
                    origin: origin,
                },
                path: "/api/"
            }
            return optionsProd;

        } else {
            const optionsProd: any = {
                origin: origin,
            }
            return optionsProd;
        }
    } else {

        let origin: string = "http://localhost:4200";

        if (type === corsType.socket) {
            const optionsDev: any = {
                cors: {
                    origin: origin,
                },
                path: "/api/"
            }
            return optionsDev;
        } else {
            const optionsDev: any = {
                origin: origin
            }
            return optionsDev;
        }

    }
};

export { corsOption, corsType }